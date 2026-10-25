/*
 * MÓDULO 27: Doom Clásico — De C a C++
 * SOLUCIÓN 09: Source Ports — Arquitectura GZDoom, Odamex, EDGE, Eternity
 */

#include "../../include/test_framework.hpp"

enum class SourcePort { GZDoom, Odamex, EDGEClassic, Eternity, ChocolateDoom };
enum class CxxStandard { C89, C99, Cpp17, Cpp20 };
enum class NetModel { PeerToPeer, ClientServerAuthoritative };

struct PortSpecs {
    CxxStandard standard;
    NetModel net;
    bool has_vm_scripting;
};

PortSpecs GetPortSpecs(SourcePort port) {
    switch (port) {
        case SourcePort::GZDoom:
            return {CxxStandard::Cpp17, NetModel::PeerToPeer, true};
        case SourcePort::Odamex:
            return {CxxStandard::Cpp17, NetModel::ClientServerAuthoritative, false};
        case SourcePort::EDGEClassic:
            return {CxxStandard::Cpp20, NetModel::PeerToPeer, true};
        case SourcePort::Eternity:
            return {CxxStandard::Cpp17, NetModel::PeerToPeer, false};
        case SourcePort::ChocolateDoom:
            return {CxxStandard::C89, NetModel::PeerToPeer, false};
    }
    return {CxxStandard::C89, NetModel::PeerToPeer, false};
}

TEST_CASE(TestPortSpecs) {
    auto gz = GetPortSpecs(SourcePort::GZDoom);
    ASSERT_EQ((int)gz.standard, (int)CxxStandard::Cpp17);
    ASSERT_TRUE(gz.has_vm_scripting);

    auto oda = GetPortSpecs(SourcePort::Odamex);
    ASSERT_EQ((int)oda.net, (int)NetModel::ClientServerAuthoritative);
    ASSERT_FALSE(oda.has_vm_scripting);

    auto edge = GetPortSpecs(SourcePort::EDGEClassic);
    ASSERT_EQ((int)edge.standard, (int)CxxStandard::Cpp20);
    ASSERT_TRUE(edge.has_vm_scripting);

    auto choc = GetPortSpecs(SourcePort::ChocolateDoom);
    ASSERT_EQ((int)choc.standard, (int)CxxStandard::C89);
    ASSERT_FALSE(choc.has_vm_scripting);
}

int main() {
    return RUN_ALL_TESTS();
}
