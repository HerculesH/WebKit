/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "ProcessCapabilities.h"

namespace WebCore {

#if USE(CG)
static bool s_HEICDecodingEnabled = false;
static bool s_AVIFDecodingEnabled = false;
static bool s_hardwareAcceleratedDecodingDisabled = false;
#endif

#if USE(CG)
void ProcessCapabilities::setHEICDecodingEnabled(bool value)
{
    s_HEICDecodingEnabled = value;
}

bool ProcessCapabilities::isHEICDecodingEnabled()
{
    return s_HEICDecodingEnabled;
}

void ProcessCapabilities::setAVIFDecodingEnabled(bool value)
{
    s_AVIFDecodingEnabled = value;
}

bool ProcessCapabilities::isAVIFDecodingEnabled()
{
    return s_AVIFDecodingEnabled;
}

void ProcessCapabilities::setHardwareAcceleratedDecodingDisabled(bool value)
{
    s_hardwareAcceleratedDecodingDisabled = value;
}

bool ProcessCapabilities::isHardwareAcceleratedDecodingDisabled()
{
    return s_hardwareAcceleratedDecodingDisabled;
}
#endif

} // namespace WebCore
