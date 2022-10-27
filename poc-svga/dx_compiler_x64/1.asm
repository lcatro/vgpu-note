;
; Input signature:
;
; Name                 Index   Mask Register SysValue  Format   Used
; -------------------- ----- ------ -------- -------- ------- ------
; no parameters
;
; Output signature:
;
; Name                 Index   Mask Register SysValue  Format   Used
; -------------------- ----- ------ -------- -------- ------- ------
; no parameters
; shader hash: 109418d2808f566154b0efd057af6284
;
; Pipeline Runtime Information: 
;
;
;
; Buffer Definitions:
;
; Resource bind info for g_data
; {
;
;   float $Element;                                   ; Offset:    0 Size:     4
;
; }
;
;
; Resource Bindings:
;
; Name                                 Type  Format         Dim      ID      HLSL Bind  Count
; ------------------------------ ---------- ------- ----------- ------- -------------- ------
; g_data                                UAV  struct         r/w      U0             u0     1
;
target datalayout = "e-m:e-p:32:32-i1:32-i8:32-i16:32-i32:32-i64:64-f16:32-f32:32-f64:64-n8:16:32:64"
target triple = "dxil-ms-dx"

%dx.types.Handle = type { i8* }
%dx.types.ResRet.f32 = type { float, float, float, float, i32 }
%"class.RWStructuredBuffer<float>" = type { float }

@"\01?sharedData@@3PAMA" = external addrspace(3) global [128 x float], align 4

define void @main() {
  %1 = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 0, i32 0, i1 false)  ; CreateHandle(resourceClass,rangeId,index,nonUniformIndex)
  %2 = call i32 @dx.op.threadIdInGroup.i32(i32 95, i32 0)  ; ThreadIdInGroup(component)
  %3 = call i32 @dx.op.groupId.i32(i32 94, i32 0)  ; GroupId(component)
  %4 = shl i32 %3, 7
  %5 = add i32 %4, %2
  %6 = call %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32 68, %dx.types.Handle %1, i32 %5, i32 0)  ; BufferLoad(srv,index,wot)
  %7 = extractvalue %dx.types.ResRet.f32 %6, 0
  %8 = getelementptr [128 x float], [128 x float] addrspace(3)* @"\01?sharedData@@3PAMA", i32 0, i32 %2
  store float %7, float addrspace(3)* %8, align 4, !tbaa !11
  call void @dx.op.barrier(i32 80, i32 9)  ; Barrier(barrierMode)
  br label %9

; <label>:9                                       ; preds = %20, %0
  %10 = phi i32 [ 1, %0 ], [ %11, %20 ]
  %11 = shl i32 %10, 1
  %12 = urem i32 %2, %11
  %13 = icmp eq i32 %12, 0
  br i1 %13, label %14, label %20

; <label>:14                                      ; preds = %9
  %15 = add i32 %10, %2
  %16 = getelementptr [128 x float], [128 x float] addrspace(3)* @"\01?sharedData@@3PAMA", i32 0, i32 %15
  %17 = load float, float addrspace(3)* %16, align 4, !tbaa !11
  %18 = load float, float addrspace(3)* %8, align 4, !tbaa !11
  %19 = fadd fast float %18, %17
  store float %19, float addrspace(3)* %8, align 4, !tbaa !11
  br label %20

; <label>:20                                      ; preds = %14, %9
  call void @dx.op.barrier(i32 80, i32 9)  ; Barrier(barrierMode)
  %21 = icmp ult i32 %11, 128
  br i1 %21, label %9, label %22

; <label>:22                                      ; preds = %20
  %23 = icmp eq i32 %2, 0
  br i1 %23, label %24, label %26

; <label>:24                                      ; preds = %22
  %25 = load float, float addrspace(3)* getelementptr inbounds ([128 x float], [128 x float] addrspace(3)* @"\01?sharedData@@3PAMA", i32 0, i32 0), align 4, !tbaa !11
  call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %1, i32 %3, i32 0, float %25, float undef, float undef, float undef, i8 1)  ; BufferStore(uav,coord0,coord1,value0,value1,value2,value3,mask)
  br label %26

; <label>:26                                      ; preds = %24, %22
  ret void
}

; Function Attrs: nounwind readnone
declare i32 @dx.op.threadIdInGroup.i32(i32, i32) #0

; Function Attrs: nounwind readnone
declare i32 @dx.op.groupId.i32(i32, i32) #0

; Function Attrs: noduplicate nounwind
declare void @dx.op.barrier(i32, i32) #1

; Function Attrs: nounwind readonly
declare %dx.types.Handle @dx.op.createHandle(i32, i8, i32, i32, i1) #2

; Function Attrs: nounwind readonly
declare %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32, %dx.types.Handle, i32, i32) #2

; Function Attrs: nounwind
declare void @dx.op.bufferStore.f32(i32, %dx.types.Handle, i32, i32, float, float, float, float, i8) #3

attributes #0 = { nounwind readnone }
attributes #1 = { noduplicate nounwind }
attributes #2 = { nounwind readonly }
attributes #3 = { nounwind }

!llvm.ident = !{!0}
!dx.version = !{!1}
!dx.valver = !{!2}
!dx.shaderModel = !{!3}
!dx.resources = !{!4}
!dx.entryPoints = !{!8}

!0 = !{!"clang version 3.7 (tags/RELEASE_370/final)"}
!1 = !{i32 1, i32 0}
!2 = !{i32 1, i32 6}
!3 = !{!"cs", i32 6, i32 0}
!4 = !{null, !5, null, null}
!5 = !{!6}
!6 = !{i32 0, %"class.RWStructuredBuffer<float>"* undef, !"", i32 0, i32 0, i32 1, i32 12, i1 false, i1 false, i1 false, !7}
!7 = !{i32 1, i32 4}
!8 = !{void ()* @main, !"main", null, !4, !9}
!9 = !{i32 0, i64 16, i32 4, !10}
!10 = !{i32 128, i32 1, i32 1}
!11 = !{!12, !12, i64 0}
!12 = !{!"float", !13, i64 0}
!13 = !{!"omnipotent char", !14, i64 0}
!14 = !{!"Simple C/C++ TBAA"}
