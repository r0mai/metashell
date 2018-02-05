//===-- SparcSubtarget.h - Define Subtarget for the SPARC -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the SPARC specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPARC_SPARCSUBTARGET_H
#define LLVM_LIB_TARGET_SPARC_SPARCSUBTARGET_H

#include "SparcFrameLowering.h"
#include "SparcISelLowering.h"
#include "SparcInstrInfo.h"
#include "llvm/CodeGen/SelectionDAGTargetInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetSubtargetInfo.h"
#include <string>

#define GET_SUBTARGETINFO_HEADER
#include "SparcGenSubtargetInfo.inc"

namespace llvm {
class StringRef;

class SparcSubtarget : public SparcGenSubtargetInfo {
  Triple TargetTriple;
  virtual void anchor();
  bool UseSoftMulDiv;
  bool IsV9;
  bool IsLeon;
  bool V8DeprecatedInsts;
  bool IsVIS, IsVIS2, IsVIS3;
  bool Is64Bit;
  bool HasHardQuad;
  bool UsePopc;
  bool UseSoftFloat;

  // LEON features
  bool HasUmacSmac;
  bool HasLeonCasa;
  bool InsertNOPLoad;
  bool FixFSMULD;
  bool ReplaceFMULS;
  bool FixAllFDIVSQRT;
  bool DetectRoundChange;
  bool PerformSDIVReplace;

  SparcInstrInfo InstrInfo;
  SparcTargetLowering TLInfo;
  SelectionDAGTargetInfo TSInfo;
  SparcFrameLowering FrameLowering;

public:
  SparcSubtarget(const Triple &TT, const std::string &CPU,
                 const std::string &FS, const TargetMachine &TM, bool is64bit);

  const SparcInstrInfo *getInstrInfo() const override { return &InstrInfo; }
  const TargetFrameLowering *getFrameLowering() const override {
    return &FrameLowering;
  }
  const SparcRegisterInfo *getRegisterInfo() const override {
    return &InstrInfo.getRegisterInfo();
  }
  const SparcTargetLowering *getTargetLowering() const override {
    return &TLInfo;
  }
  const SelectionDAGTargetInfo *getSelectionDAGInfo() const override {
    return &TSInfo;
  }

  bool enableMachineScheduler() const override;

  bool useSoftMulDiv() const { return UseSoftMulDiv; }
  bool isV9() const { return IsV9; }
  bool isLeon() const { return IsLeon; }
  bool isVIS() const { return IsVIS; }
  bool isVIS2() const { return IsVIS2; }
  bool isVIS3() const { return IsVIS3; }
  bool useDeprecatedV8Instructions() const { return V8DeprecatedInsts; }
  bool hasHardQuad() const { return HasHardQuad; }
  bool usePopc() const { return UsePopc; }
  bool useSoftFloat() const { return UseSoftFloat; }

  // Leon options
  bool hasUmacSmac() const { return HasUmacSmac; }
  bool performSDIVReplace() const { return PerformSDIVReplace; }
  bool hasLeonCasa() const { return HasLeonCasa; }
  bool insertNOPLoad() const { return InsertNOPLoad; }
  bool fixFSMULD() const { return FixFSMULD; }
  bool replaceFMULS() const { return ReplaceFMULS; }
  bool fixAllFDIVSQRT() const { return FixAllFDIVSQRT; }
  bool detectRoundChange() const { return DetectRoundChange; }

  /// ParseSubtargetFeatures - Parses features string setting specified
  /// subtarget options.  Definition of function is auto generated by tblgen.
  void ParseSubtargetFeatures(StringRef CPU, StringRef FS);
  SparcSubtarget &initializeSubtargetDependencies(StringRef CPU, StringRef FS);

  bool is64Bit() const { return Is64Bit; }

  /// The 64-bit ABI uses biased stack and frame pointers, so the stack frame
  /// of the current function is the area from [%sp+BIAS] to [%fp+BIAS].
  int64_t getStackPointerBias() const {
    return is64Bit() ? 2047 : 0;
  }

  /// Given a actual stack size as determined by FrameInfo, this function
  /// returns adjusted framesize which includes space for register window
  /// spills and arguments.
  int getAdjustedFrameSize(int stackSize) const;

  bool isTargetLinux() const { return TargetTriple.isOSLinux(); }
};

} // end namespace llvm

#endif
