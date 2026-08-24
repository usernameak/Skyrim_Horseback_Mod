// Horseback -- mod for The Elder Scrolls V: Skyrim that extends
// horseback riding features.
// Copyright (C) 2023-2026  usernameak
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cstddef>
#include <SKSE/SKSE.h>

#include <xbyak/xbyak.h>

// code in po3helpers namespace is originally under MIT license,
// Copyright (c) 2021-2026 powerofthree

// various helpers borrowed from po3's Papyrus Extender

namespace po3helpers {
    template <class T, std::size_t BYTES>
    void HookFunctionPrologue(std::uintptr_t a_src) {
        struct Patch : Xbyak::CodeGenerator {
            Patch(std::uintptr_t a_originalFuncAddr, std::size_t a_originalByteLength) {
                // Hook returns here. Execute the restored bytes and jump back to the original function.
                for (size_t i = 0; i < a_originalByteLength; ++i) {
                    db(*reinterpret_cast<std::uint8_t *>(a_originalFuncAddr + i));
                }

                jmp(qword[rip]);
                dq(a_originalFuncAddr + a_originalByteLength);
            }
        };

        Patch p(a_src, BYTES);
        p.ready();

        auto &trampoline = SKSE::GetTrampoline();
        trampoline.write_branch<5>(a_src, T::thunk);

        auto alloc = trampoline.allocate(p.getSize());
        std::memcpy(alloc, p.getCode(), p.getSize());

        T::func = reinterpret_cast<std::uintptr_t>(alloc);
    }
}
