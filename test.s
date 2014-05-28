; ModuleID = 'tests/test.c'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-redhat-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
  %1 = call i32 @fun(i32 10)
  ret i32 0
}

; Function Attrs: nounwind uwtable
define i32 @fun(i32 %c) #0 {
  %1 = alloca i32, align 4
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  store i32 %c, i32* %1, align 4
  br label %2

; <label>:2                                       ; preds = %0
  store i32 0, i32* %a, align 4
  br label %3

; <label>:3                                       ; preds = %16, %2
  %4 = load i32* %a, align 4
  %5 = add nsw i32 %4, 1
  store i32 %5, i32* %b, align 4
  br label %6

; <label>:6                                       ; preds = %3
  %7 = load i32* %1, align 4
  %8 = load i32* %b, align 4
  %9 = add nsw i32 %7, %8
  store i32 %9, i32* %1, align 4
  br label %10

; <label>:10                                      ; preds = %6
  %11 = load i32* %b, align 4
  %12 = mul nsw i32 %11, 2
  store i32 %12, i32* %a, align 4
  br label %13

; <label>:13                                      ; preds = %10
  %14 = load i32* %a, align 4
  %15 = icmp slt i32 %14, 100
  br i1 %15, label %16, label %17

; <label>:16                                      ; preds = %13
  br label %3

; <label>:17                                      ; preds = %13
  br label %18

; <label>:18                                      ; preds = %17
  %19 = load i32* %1, align 4
  ret i32 %19
}

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
