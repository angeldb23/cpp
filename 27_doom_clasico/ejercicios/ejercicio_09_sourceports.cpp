/*
 * MÓDULO 27: Doom Clásico — De C a C++
 * EJERCICIO 09: Source Ports — Arquitectura GZDoom, Odamex, EDGE, Eternity
 *
 * BASADO EN: Lección 09 (source_ports) y Lección 13 (source_ports_profundidad)
 *
 * OBJETIVO: Implementar la consulta de especificaciones técnicas y estándares
 * de C++ de los principales source ports del motor Doom.
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
    // TODO: devolver el estándar de C/C++, modelo de red y si tiene VM de scripting
    // para GZDoom (C++17, P2P, ZScript), Odamex (C++17, C/S, sin VM),
    // EDGEClassic (C++20, P2P, COAL) y ChocolateDoom (C89, P2P, sin VM)
    (void)port;
    return {CxxStandard::C89, NetModel::PeerToPeer, false};
}

// ============================================================================
// TESTS (NO MODIFICAR)
// ============================================================================

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
