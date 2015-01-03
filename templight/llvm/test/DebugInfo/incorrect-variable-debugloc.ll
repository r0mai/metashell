; REQUIRES: object-emission

; RUN: %llc_dwarf -O2 -filetype=obj < %s | llvm-dwarfdump -debug-dump=info - | FileCheck %s

; This is a test case that's as reduced as I can get it, though I haven't fully
; understood the mechanisms by which this bug occurs, so perhaps there's further
; simplification to be had (it's certainly a bit non-obvious what's going on). I
; hesitate to hand-craft or otherwise simplify the IR compared to what Clang
; generates as this is a particular tickling of optimizations and debug location
; propagation I want a realistic example of.

; Generated with clang-tot -cc1 -g -O2 -w -std=c++11  -fsanitize=address,use-after-return -fcxx-exceptions -fexceptions -x c++ incorrect-variable-debug-loc.cpp -emit-llvm

; struct A {
;   int m_fn1();
; };
;
; struct B {
;   void __attribute__((always_inline)) m_fn2() { i = 0; }
;   int i;
; };
;
; struct C {
;   void m_fn3();
;   int j;
;   B b;
; };
;
; int fn1() {
;   C A;
;   A.b.m_fn2();
;   A.m_fn3();
; }
; void C::m_fn3() {
;   A().m_fn1();
;   b.m_fn2();
; }

; CHECK: DW_TAG_structure_type
; CHECK-NEXT: DW_AT_name {{.*}} "C"
; CHECK:   DW_TAG_subprogram
; CHECK-NOT: DW_TAG
; CHECK: DW_AT_name {{.*}} "m_fn3"

; CHECK: DW_AT_specification {{.*}} "_ZN1C5m_fn3Ev"
; CHECK-NOT: DW_TAG
; CHECK:   DW_TAG_formal_parameter
; CHECK-NOT: DW_TAG
; CHECK:     DW_AT_name {{.*}} "this"

%struct.C = type { i32, %struct.B }
%struct.B = type { i32 }
%struct.A = type { i8 }

@llvm.global_ctors = appending global [1 x { i32, void ()* }] [{ i32, void ()* } { i32 1, void ()* @asan.module_ctor }]
@__asan_option_detect_stack_use_after_return = external global i32
@__asan_gen_ = private unnamed_addr constant [11 x i8] c"1 32 8 1 A\00", align 1
@__asan_gen_1 = private unnamed_addr constant [13 x i8] c"1 32 1 3 tmp\00", align 1

; Function Attrs: noreturn sanitize_address
define i32 @_Z3fn1v() #0 {
entry:
  %MyAlloca = alloca [64 x i8], align 32, !dbg !39
  %0 = ptrtoint [64 x i8]* %MyAlloca to i64, !dbg !39
  %1 = load i32* @__asan_option_detect_stack_use_after_return, !dbg !39
  %2 = icmp ne i32 %1, 0, !dbg !39
  br i1 %2, label %3, label %5

; <label>:3                                       ; preds = %entry
  %4 = call i64 @__asan_stack_malloc_0(i64 64, i64 %0), !dbg !39
  br label %5

; <label>:5                                       ; preds = %entry, %3
  %6 = phi i64 [ %0, %entry ], [ %4, %3 ], !dbg !39
  %7 = add i64 %6, 32, !dbg !39
  %8 = inttoptr i64 %7 to %struct.C*, !dbg !39
  %9 = inttoptr i64 %6 to i64*, !dbg !39
  store i64 1102416563, i64* %9, !dbg !39
  %10 = add i64 %6, 8, !dbg !39
  %11 = inttoptr i64 %10 to i64*, !dbg !39
  store i64 ptrtoint ([11 x i8]* @__asan_gen_ to i64), i64* %11, !dbg !39
  %12 = add i64 %6, 16, !dbg !39
  %13 = inttoptr i64 %12 to i64*, !dbg !39
  store i64 ptrtoint (i32 ()* @_Z3fn1v to i64), i64* %13, !dbg !39
  %14 = lshr i64 %6, 3, !dbg !39
  %15 = add i64 %14, 2147450880, !dbg !39
  %16 = add i64 %15, 0, !dbg !39
  %17 = inttoptr i64 %16 to i64*, !dbg !39
  store i64 -868083117767659023, i64* %17, !dbg !39
  %i.i = getelementptr inbounds %struct.C* %8, i64 0, i32 1, i32 0, !dbg !39
  %18 = ptrtoint i32* %i.i to i64, !dbg !39
  %19 = lshr i64 %18, 3, !dbg !39
  %20 = add i64 %19, 2147450880, !dbg !39
  %21 = inttoptr i64 %20 to i8*, !dbg !39
  %22 = load i8* %21, !dbg !39
  %23 = icmp ne i8 %22, 0, !dbg !39
  br i1 %23, label %24, label %30, !dbg !39

; <label>:24                                      ; preds = %5
  %25 = and i64 %18, 7, !dbg !39
  %26 = add i64 %25, 3, !dbg !39
  %27 = trunc i64 %26 to i8, !dbg !39
  %28 = icmp sge i8 %27, %22, !dbg !39
  br i1 %28, label %29, label %30

; <label>:29                                      ; preds = %24
  call void @__asan_report_store4(i64 %18), !dbg !39
  call void asm sideeffect "", ""()
  unreachable

; <label>:30                                      ; preds = %24, %5
  store i32 0, i32* %i.i, align 4, !dbg !39, !tbaa !41
  tail call void @llvm.dbg.value(metadata %struct.C* %8, i64 0, metadata !27, metadata !{!"0x102"}), !dbg !46
  call void @_ZN1C5m_fn3Ev(%struct.C* %8), !dbg !47
  unreachable, !dbg !47
}

; Function Attrs: sanitize_address
define void @_ZN1C5m_fn3Ev(%struct.C* nocapture %this) #1 align 2 {
entry:
  %MyAlloca = alloca [64 x i8], align 32, !dbg !48
  %0 = ptrtoint [64 x i8]* %MyAlloca to i64, !dbg !48
  %1 = load i32* @__asan_option_detect_stack_use_after_return, !dbg !48
  %2 = icmp ne i32 %1, 0, !dbg !48
  br i1 %2, label %3, label %5

; <label>:3                                       ; preds = %entry
  %4 = call i64 @__asan_stack_malloc_0(i64 64, i64 %0), !dbg !48
  br label %5

; <label>:5                                       ; preds = %entry, %3
  %6 = phi i64 [ %0, %entry ], [ %4, %3 ], !dbg !48
  %7 = add i64 %6, 32, !dbg !48
  %8 = inttoptr i64 %7 to %struct.A*, !dbg !48
  %9 = inttoptr i64 %6 to i64*, !dbg !48
  store i64 1102416563, i64* %9, !dbg !48
  %10 = add i64 %6, 8, !dbg !48
  %11 = inttoptr i64 %10 to i64*, !dbg !48
  store i64 ptrtoint ([13 x i8]* @__asan_gen_1 to i64), i64* %11, !dbg !48
  %12 = add i64 %6, 16, !dbg !48
  %13 = inttoptr i64 %12 to i64*, !dbg !48
  store i64 ptrtoint (void (%struct.C*)* @_ZN1C5m_fn3Ev to i64), i64* %13, !dbg !48
  %14 = lshr i64 %6, 3, !dbg !48
  %15 = add i64 %14, 2147450880, !dbg !48
  %16 = add i64 %15, 0, !dbg !48
  %17 = inttoptr i64 %16 to i64*, !dbg !48
  store i64 -868083113472691727, i64* %17, !dbg !48
  tail call void @llvm.dbg.value(metadata %struct.C* %this, i64 0, metadata !30, metadata !{!"0x102"}), !dbg !48
  %call = call i32 @_ZN1A5m_fn1Ev(%struct.A* %8), !dbg !49
  %i.i = getelementptr inbounds %struct.C* %this, i64 0, i32 1, i32 0, !dbg !50
  %18 = ptrtoint i32* %i.i to i64, !dbg !50
  %19 = lshr i64 %18, 3, !dbg !50
  %20 = add i64 %19, 2147450880, !dbg !50
  %21 = inttoptr i64 %20 to i8*, !dbg !50
  %22 = load i8* %21, !dbg !50
  %23 = icmp ne i8 %22, 0, !dbg !50
  br i1 %23, label %24, label %30, !dbg !50

; <label>:24                                      ; preds = %5
  %25 = and i64 %18, 7, !dbg !50
  %26 = add i64 %25, 3, !dbg !50
  %27 = trunc i64 %26 to i8, !dbg !50
  %28 = icmp sge i8 %27, %22, !dbg !50
  br i1 %28, label %29, label %30

; <label>:29                                      ; preds = %24
  call void @__asan_report_store4(i64 %18), !dbg !50
  call void asm sideeffect "", ""()
  unreachable

; <label>:30                                      ; preds = %24, %5
  store i32 0, i32* %i.i, align 4, !dbg !50, !tbaa !41
  store i64 1172321806, i64* %9, !dbg !52
  %31 = icmp ne i64 %6, %0, !dbg !52
  br i1 %31, label %32, label %39, !dbg !52

; <label>:32                                      ; preds = %30
  %33 = add i64 %15, 0, !dbg !52
  %34 = inttoptr i64 %33 to i64*, !dbg !52
  store i64 -723401728380766731, i64* %34, !dbg !52
  %35 = add i64 %6, 56, !dbg !52
  %36 = inttoptr i64 %35 to i64*, !dbg !52
  %37 = load i64* %36, !dbg !52
  %38 = inttoptr i64 %37 to i8*, !dbg !52
  store i8 0, i8* %38, !dbg !52
  br label %42, !dbg !52

; <label>:39                                      ; preds = %30
  %40 = add i64 %15, 0, !dbg !52
  %41 = inttoptr i64 %40 to i64*, !dbg !52
  store i64 0, i64* %41, !dbg !52
  br label %42, !dbg !52

; <label>:42                                      ; preds = %39, %32
  ret void, !dbg !52
}

declare i32 @_ZN1A5m_fn1Ev(%struct.A*) #2

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #3

define internal void @asan.module_ctor() {
  tail call void @__asan_init_v3()
  ret void
}

declare void @__asan_init_v3()

declare void @__asan_report_load1(i64)

declare void @__asan_load1(i64)

declare void @__asan_report_load2(i64)

declare void @__asan_load2(i64)

declare void @__asan_report_load4(i64)

declare void @__asan_load4(i64)

declare void @__asan_report_load8(i64)

declare void @__asan_load8(i64)

declare void @__asan_report_load16(i64)

declare void @__asan_load16(i64)

declare void @__asan_report_store1(i64)

declare void @__asan_store1(i64)

declare void @__asan_report_store2(i64)

declare void @__asan_store2(i64)

declare void @__asan_report_store4(i64)

declare void @__asan_store4(i64)

declare void @__asan_report_store8(i64)

declare void @__asan_store8(i64)

declare void @__asan_report_store16(i64)

declare void @__asan_store16(i64)

declare void @__asan_report_load_n(i64, i64)

declare void @__asan_report_store_n(i64, i64)

declare void @__asan_loadN(i64, i64)

declare void @__asan_storeN(i64, i64)

declare i8* @__asan_memmove(i8*, i8*, i64)

declare i8* @__asan_memcpy(i8*, i8*, i64)

declare i8* @__asan_memset(i8*, i32, i64)

declare void @__asan_handle_no_return()

declare void @__sanitizer_cov()

declare void @__sanitizer_ptr_cmp(i64, i64)

declare void @__sanitizer_ptr_sub(i64, i64)

declare i64 @__asan_stack_malloc_0(i64, i64)

declare void @__asan_stack_free_0(i64, i64, i64)

declare i64 @__asan_stack_malloc_1(i64, i64)

declare void @__asan_stack_free_1(i64, i64, i64)

declare i64 @__asan_stack_malloc_2(i64, i64)

declare void @__asan_stack_free_2(i64, i64, i64)

declare i64 @__asan_stack_malloc_3(i64, i64)

declare void @__asan_stack_free_3(i64, i64, i64)

declare i64 @__asan_stack_malloc_4(i64, i64)

declare void @__asan_stack_free_4(i64, i64, i64)

declare i64 @__asan_stack_malloc_5(i64, i64)

declare void @__asan_stack_free_5(i64, i64, i64)

declare i64 @__asan_stack_malloc_6(i64, i64)

declare void @__asan_stack_free_6(i64, i64, i64)

declare i64 @__asan_stack_malloc_7(i64, i64)

declare void @__asan_stack_free_7(i64, i64, i64)

declare i64 @__asan_stack_malloc_8(i64, i64)

declare void @__asan_stack_free_8(i64, i64, i64)

declare i64 @__asan_stack_malloc_9(i64, i64)

declare void @__asan_stack_free_9(i64, i64, i64)

declare i64 @__asan_stack_malloc_10(i64, i64)

declare void @__asan_stack_free_10(i64, i64, i64)

declare void @__asan_poison_stack_memory(i64, i64)

declare void @__asan_unpoison_stack_memory(i64, i64)

declare void @__asan_before_dynamic_init(i64)

declare void @__asan_after_dynamic_init()

declare void @__asan_register_globals(i64, i64)

declare void @__asan_unregister_globals(i64, i64)

declare void @__sanitizer_cov_module_init(i64)

attributes #0 = { noreturn sanitize_address "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { sanitize_address "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!36, !37}
!llvm.ident = !{!38}

!0 = !{!"0x11\004\00clang version 3.5.0 \001\00\000\00\001", !1, !2, !3, !21, !2, !2} ; [ DW_TAG_compile_unit ] [/tmp/dbginfo/<stdin>] [DW_LANG_C_plus_plus]
!1 = !{!"<stdin>", !"/tmp/dbginfo"}
!2 = !{}
!3 = !{!4, !14}
!4 = !{!"0x13\00C\0010\0064\0032\000\000\000", !5, null, null, !6, null, null, !"_ZTS1C"} ; [ DW_TAG_structure_type ] [C] [line 10, size 64, align 32, offset 0] [def] [from ]
!5 = !{!"incorrect-variable-debug-loc.cpp", !"/tmp/dbginfo"}
!6 = !{!7, !9, !10}
!7 = !{!"0xd\00j\0012\0032\0032\000\000", !5, !"_ZTS1C", !8} ; [ DW_TAG_member ] [j] [line 12, size 32, align 32, offset 0] [from int]
!8 = !{!"0x24\00int\000\0032\0032\000\000\005", null, null} ; [ DW_TAG_base_type ] [int] [line 0, size 32, align 32, offset 0, enc DW_ATE_signed]
!9 = !{!"0xd\00b\0013\0032\0032\0032\000", !5, !"_ZTS1C", !"_ZTS1B"} ; [ DW_TAG_member ] [b] [line 13, size 32, align 32, offset 32] [from _ZTS1B]
!10 = !{!"0x2e\00m_fn3\00m_fn3\00_ZN1C5m_fn3Ev\0011\000\000\000\006\00256\001\0011", !5, !"_ZTS1C", !11, null, null, null, i32 0, null} ; [ DW_TAG_subprogram ] [line 11] [m_fn3]
!11 = !{!"0x15\00\000\000\000\000\000\000", i32 0, null, null, !12, null, null, null} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!12 = !{null, !13}
!13 = !{!"0xf\00\000\0064\0064\000\001088", null, null, !"_ZTS1C"} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 64, offset 0] [artificial] [from _ZTS1C]
!14 = !{!"0x13\00B\005\0032\0032\000\000\000", !5, null, null, !15, null, null, !"_ZTS1B"} ; [ DW_TAG_structure_type ] [B] [line 5, size 32, align 32, offset 0] [def] [from ]
!15 = !{!16, !17}
!16 = !{!"0xd\00i\007\0032\0032\000\000", !5, !"_ZTS1B", !8} ; [ DW_TAG_member ] [i] [line 7, size 32, align 32, offset 0] [from int]
!17 = !{!"0x2e\00m_fn2\00m_fn2\00_ZN1B5m_fn2Ev\006\000\000\000\006\00256\001\006", !5, !"_ZTS1B", !18, null, null, null, i32 0, null} ; [ DW_TAG_subprogram ] [line 6] [m_fn2]
!18 = !{!"0x15\00\000\000\000\000\000\000", i32 0, null, null, !19, null, null, null} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!19 = !{null, !20}
!20 = !{!"0xf\00\000\0064\0064\000\001088", null, null, !"_ZTS1B"} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 64, offset 0] [artificial] [from _ZTS1B]
!21 = !{!22, !28, !32}
!22 = !{!"0x2e\00fn1\00fn1\00_Z3fn1v\0016\000\001\000\006\00256\001\0016", !5, !23, !24, null, i32 ()* @_Z3fn1v, null, null, !26} ; [ DW_TAG_subprogram ] [line 16] [def] [fn1]
!23 = !{!"0x29", !5}         ; [ DW_TAG_file_type ] [/tmp/dbginfo/incorrect-variable-debug-loc.cpp]
!24 = !{!"0x15\00\000\000\000\000\000\000", i32 0, null, null, !25, null, null, null} ; [ DW_TAG_subroutine_type ] [line 0, size 0, align 0, offset 0] [from ]
!25 = !{!8}
!26 = !{!27}
!27 = !{!"0x100\00A\0017\000", !22, !23, !"_ZTS1C"} ; [ DW_TAG_auto_variable ] [A] [line 17]
!28 = !{!"0x2e\00m_fn3\00m_fn3\00_ZN1C5m_fn3Ev\0021\000\001\000\006\00256\001\0021", !5, !"_ZTS1C", !11, null, void (%struct.C*)* @_ZN1C5m_fn3Ev, null, !10, !29} ; [ DW_TAG_subprogram ] [line 21] [def] [m_fn3]
!29 = !{!30}
!30 = !{!"0x101\00this\0016777216\001088", !28, null, !31} ; [ DW_TAG_arg_variable ] [this] [line 0]
!31 = !{!"0xf\00\000\0064\0064\000\000", null, null, !"_ZTS1C"} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 64, offset 0] [from _ZTS1C]
!32 = !{!"0x2e\00m_fn2\00m_fn2\00_ZN1B5m_fn2Ev\006\000\001\000\006\00256\001\006", !5, !"_ZTS1B", !18, null, null, null, !17, !33} ; [ DW_TAG_subprogram ] [line 6] [def] [m_fn2]
!33 = !{!34}
!34 = !{!"0x101\00this\0016777216\001088", !32, null, !35} ; [ DW_TAG_arg_variable ] [this] [line 0]
!35 = !{!"0xf\00\000\0064\0064\000\000", null, null, !"_ZTS1B"} ; [ DW_TAG_pointer_type ] [line 0, size 64, align 64, offset 0] [from _ZTS1B]
!36 = !{i32 2, !"Dwarf Version", i32 4}
!37 = !{i32 2, !"Debug Info Version", i32 2}
!38 = !{!"clang version 3.5.0 "}
!39 = !{i32 6, i32 0, !32, !40}
!40 = !{i32 18, i32 0, !22, null}
!41 = !{!42, !43, i64 0}
!42 = !{!"_ZTS1B", !43, i64 0}
!43 = !{!"int", !44, i64 0}
!44 = !{!"omnipotent char", !45, i64 0}
!45 = !{!"Simple C/C++ TBAA"}
!46 = !{i32 17, i32 0, !22, null}
!47 = !{i32 19, i32 0, !22, null}
!48 = !{i32 0, i32 0, !28, null}
!49 = !{i32 22, i32 0, !28, null}
!50 = !{i32 6, i32 0, !32, !51}
!51 = !{i32 23, i32 0, !28, null}
!52 = !{i32 24, i32 0, !28, null}
