; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc -mcpu=zEC12 -o - %s | FileCheck %s

target triple = "s390x-ibm-linux"

define <2 x float> @pr32505(<2 x i8> * %a) {
; CHECK-LABEL: pr32505:
; CHECK:       # %bb.0:
; CHECK-NEXT:    lbh %r0, 1(%r2)
; CHECK-NEXT:    lbh %r1, 0(%r2)
; CHECK-NEXT:    ldgr %f0, %r1
; CHECK-NEXT:    ldgr %f2, %r0
; CHECK-NEXT:    # kill: def %f0s killed %f0s killed %f0d
; CHECK-NEXT:    # kill: def %f2s killed %f2s killed %f2d
; CHECK-NEXT:    br %r14
  %L17 = load <2 x i8>, <2 x i8>* %a
  %Se21 = sext <2 x i8> %L17 to <2 x i32>
  %BC = bitcast <2 x i32> %Se21 to <2 x float>
  ret <2 x float> %BC
}
