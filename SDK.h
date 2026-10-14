#pragma once
#include "Memory.h"
#include "Offsets.hpp"
#include <cmath>
#include <string>
#include <vector>

// ---------------------------------------------------------------- math ---
struct Vec2 { float x = 0, y = 0; };
struct Vec3
{
    float x = 0, y = 0, z = 0;
    Vec3 operator+(const Vec3& o) const { return { x + o.x, y + o.y, z + o.z }; }
    Vec3 operator-(const Vec3& o) const { return { x - o.x, y - o.y, z - o.z }; }
    Vec3 operator*(float s)       const { return { x * s, y * s, z * s }; }
    float Length() const { return std::sqrt(x * x + y * y + z * z); }
    Vec3 Normalized() const { float l = Length(); return l > 1e-6f ? (*this) * (1.0f / l) : Vec3{}; }
    float Dist(const Vec3& o) const { return (*this - o).Length(); }
};

struct Matrix4x4 { float m[4][4]; };

inline Vec2 WorldToScreen(const Matrix4x4& vm, const Vec3& world, int screenW, int screenH)
{
    // viewmatrix is row-major; clip.w from the last row
    float w = vm.m[3][0] * world.x + vm.m[3][1] * world.y + vm.m[3][2] * world.z + vm.m[3][3];
    Vec2 out{ -1, -1 };
    if (w < 0.1f)
        return out;
    float inv = 1.0f / w;
    float nx = (vm.m[0][0] * world.x + vm.m[0][1] * world.y + vm.m[0][2] * world.z + vm.m[0][3]) * inv;
    float ny = (vm.m[1][0] * world.x + vm.m[1][1] * world.y + vm.m[1][2] * world.z + vm.m[1][3]) * inv;
    out.x = (nx + 1.0f) * 0.5f * static_cast<float>(screenW);
    out.y = (1.0f - ny) * 0.5f * static_cast<float>(screenH);
    return out;
}

// Camera forward/right/up pulled straight from the view matrix rows.
inline Vec3 CameraForward(const Matrix4x4& vm)
{
    Vec3 f{ -vm.m[0][2], -vm.m[1][2], -vm.m[2][2] };
    return f.Normalized();
}
inline Vec3 CameraRight(const Matrix4x4& vm)
{
    Vec3 r{ vm.m[0][0], vm.m[1][0], vm.m[2][0] };
    return r.Normalized();
}

// ---------------------------------------------------------- instance ----
class Instance
{
public:
    Instance() = default;
    Instance(const Memory* mem, uintptr_t addr) : m_mem(mem), m_addr(addr) {}

    bool Valid() const { return m_mem && m_mem->IsAttached() && m_addr != 0; }
    uintptr_t Addr() const { return m_addr; }

    std::string Name() const
    {
        return Valid() ? m_mem->ReadRbxString(m_addr + Offsets::Instance_Name) : std::string{};
    }

    std::string ClassName() const
    {
        if (!Valid()) return {};
        uintptr_t desc = m_mem->Read<uintptr_t>(m_addr + Offsets::Instance_ClassDesc);
        if (!desc) return {};
        return m_mem->ReadRbxString(desc + Offsets::ClassDesc_ClassName);
    }

    std::vector<Instance> Children() const
    {
        std::vector<Instance> out;
        if (!Valid()) return out;
        uintptr_t first = m_mem->Read<uintptr_t>(m_addr + Offsets::Instance_ChildrenStart);
        uintptr_t last  = m_mem->Read<uintptr_t>(m_addr + Offsets::Instance_ChildrenEnd);
        if (!first || !last || last <= first)
            return out;
        size_t count = (last - first) / sizeof(uintptr_t);
        if (count == 0 || count > 4096) // sanity: datamodel children are in the dozens
            return out;
        std::vector<uintptr_t> addrs(count);
        if (!m_mem->ReadRaw(first, addrs.data(), count * sizeof(uintptr_t)))
            return {};
        out.reserve(count);
        for (uintptr_t a : addrs)
            if (a) out.emplace_back(m_mem, a);
        return out;
    }

    Instance FindFirstChild(const std::string& name, bool recursive = false, int depth = 0) const
    {
        if (!Valid() || depth > 6) return {};
        for (const auto& c : Children())
        {
            if (c.Name() == name)
                return c;
            if (recursive)
            {
                Instance r = c.FindFirstChild(name, true, depth + 1);
                if (r.Valid()) return r;
            }
        }
        return {};
    }

    Instance FindFirstChildOfClass(const std::string& className, bool recursive = false, int depth = 0) const
    {
        if (!Valid() || depth > 6) return {};
        for (const auto& c : Children())
        {
            if (c.ClassName() == className)
                return c;
            if (recursive)
            {
                Instance r = c.FindFirstChildOfClass(className, true, depth + 1);
                if (r.Valid()) return r;
            }
        }
        return {};
    }

    // Every descendant BasePart (Part/MeshPart) under this instance.
    void CollectParts(std::vector<Instance>& out, int depth = 0) const
    {
        if (!Valid() || depth > 6) return;
        for (const auto& c : Children())
        {
            std::string cn = c.ClassName();
            if (cn == "Part" || cn == "MeshPart")
                out.push_back(c);
            c.CollectParts(out, depth + 1);
        }
    }

    Vec3 PrimitivePosition() const
    {
        if (!Valid()) return {};
        uintptr_t prim = m_mem->Read<uintptr_t>(m_addr + Offsets::BasePart_Primitive);
        if (!prim) return {};
        return m_mem->Read<Vec3>(prim + Offsets::Primitive_Position);
    }

    bool SetPrimitivePosition(const Vec3& pos) const
    {
        if (!Valid()) return false;
        uintptr_t prim = m_mem->Read<uintptr_t>(m_addr + Offsets::BasePart_Primitive);
        return prim && m_mem->Write<Vec3>(prim + Offsets::Primitive_Position, pos);
    }

    bool SetCanCollide(bool collide) const
    {
        if (!Valid()) return false;
        uintptr_t prim = m_mem->Read<uintptr_t>(m_addr + Offsets::BasePart_Primitive);
        if (!prim) return false;
        uint8_t flags = m_mem->Read<uint8_t>(prim + Offsets::Primitive_Flags);
        uint8_t next = collide ? (flags | Offsets::PrimitiveFlag_CanCollide)
                               : (flags & ~Offsets::PrimitiveFlag_CanCollide);
        return m_mem->Write<uint8_t>(prim + Offsets::Primitive_Flags, next);
    }

private:
    const Memory* m_mem = nullptr;
    uintptr_t m_addr = 0;
};

// ------------------------------------------------------------- entity ----
struct Entity
{
    uintptr_t player = 0;
    uintptr_t model = 0;
    uintptr_t humanoid = 0;
    uintptr_t hrp = 0; // HumanoidRootPart instance
    std::string name;
    std::string displayName;
    uintptr_t team = 0;
    int userId = 0;
    float health = 0;
    float maxHealth = 100;
    Vec3 hrpPos{};
    Vec3 headPos{};
    Vec3 torsoPos{};
    bool valid = false;

    bool Alive() const { return valid && health > 0.5f; }
};

// Resolves the live object graph and maintains the entity cache.
class Roblox
{
public:
    explicit Roblox(const Memory* mem) : m_mem(mem) {}

    // Walks: base -> FakeDataModel -> DataModel -> Workspace/Players.
    // Call periodically; cheap enough to run every ~250ms.
    bool Resolve()
    {
        if (!m_mem || !m_mem->IsAttached())
            return false;

        uintptr_t fake = m_mem->Read<uintptr_t>(m_mem->Base() + Offsets::FakeDataModel_Pointer);
        if (!fake) return false;
        m_dataModel = Instance(m_mem, m_mem->Read<uintptr_t>(fake + Offsets::FakeDataModel_RealData));
        if (!m_dataModel.Valid()) return false;

        m_workspace = Instance(m_mem, m_mem->Read<uintptr_t>(m_dataModel.Addr() + Offsets::DataModel_Workspace));
        m_players   = m_dataModel.FindFirstChildOfClass("Players");
        if (!m_players.Valid()) return false;

        uintptr_t local = m_mem->Read<uintptr_t>(m_players.Addr() + Offsets::Players_LocalPlayer);
        m_localPlayer = Instance(m_mem, local);

        m_camera = Instance(m_mem, m_mem->Read<uintptr_t>(m_workspace.Addr() + Offsets::Workspace_CurrentCamera));

        uintptr_t ve = m_mem->Read<uintptr_t>(m_mem->Base() + Offsets::VisualEngine_Pointer);
        m_viewMatrix = ve ? m_mem->Read<Matrix4x4>(ve + Offsets::VisualEngine_ViewMatrix) : Matrix4x4{};
        m_hasMatrix = ve != 0;
        return true;
    }

    // Rebuilds the entity list from the Players service.
    void UpdateEntities()
    {
        std::vector<Entity> fresh;
        if (!m_players.Valid())
        {
            m_entities.swap(fresh);
            return;
        }

        uintptr_t localAddr = m_localPlayer.Addr();
        for (const auto& p : m_players.Children())
        {
            if (p.Addr() == localAddr)
                continue; // local player handled separately
            Entity e = ReadEntity(p);
            if (e.valid)
                fresh.push_back(std::move(e));
        }
        m_local = ReadEntity(m_localPlayer);
        m_entities.swap(fresh);
    }

    const std::vector<Entity>& Entities() const { return m_entities; }
    const Entity& Local() const { return m_local; }
    const Matrix4x4& ViewMatrix() const { return m_viewMatrix; }
    bool HasMatrix() const { return m_hasMatrix; }
    Instance Camera() const { return m_camera; }
    Vec3 CameraPosition() const
    {
        return m_camera.Valid() ? m_mem->Read<Vec3>(m_camera.Addr() + Offsets::Camera_Position) : Vec3{};
    }

private:
    Entity ReadEntity(const Instance& player) const
    {
        Entity e;
        if (!player.Valid())
            return e;
        e.player = player.Addr();
        e.name = player.Name();
        e.displayName = m_mem->ReadRbxString(player.Addr() + Offsets::Player_DisplayName);
        e.team = m_mem->Read<uintptr_t>(player.Addr() + Offsets::Player_Team);
        e.userId = m_mem->Read<int>(player.Addr() + Offsets::Player_UserId);

        Instance model(m_mem, m_mem->Read<uintptr_t>(player.Addr() + Offsets::Player_ModelInstance));
        if (!model.Valid())
            return e;
        e.model = model.Addr();

        Instance humanoid = model.FindFirstChildOfClass("Humanoid");
        if (!humanoid.Valid())
            return e;
        e.humanoid = humanoid.Addr();
        e.health = m_mem->Read<float>(humanoid.Addr() + Offsets::Humanoid_Health);
        e.maxHealth = m_mem->Read<float>(humanoid.Addr() + Offsets::Humanoid_MaxHealth);

        Instance hrp(m_mem, m_mem->Read<uintptr_t>(humanoid.Addr() + Offsets::Humanoid_HRP));
        if (hrp.Valid())
        {
            e.hrp = hrp.Addr();
            e.hrpPos = hrp.PrimitivePosition();
        }

        Instance head = model.FindFirstChild("Head");
        if (head.Valid())
            e.headPos = head.PrimitivePosition();

        Instance torso = model.FindFirstChild("UpperTorso");
        if (!torso.Valid())
            torso = model.FindFirstChild("Torso");
        if (torso.Valid())
            e.torsoPos = torso.PrimitivePosition();

        e.valid = e.hrp != 0;
        return e;
    }

    const Memory* m_mem = nullptr;
    Instance m_dataModel, m_workspace, m_players, m_localPlayer, m_camera;
    Matrix4x4 m_viewMatrix{};
    bool m_hasMatrix = false;
    std::vector<Entity> m_entities;
    Entity m_local;
};
