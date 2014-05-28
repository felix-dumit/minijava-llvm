; ModuleID = 'fac.opt'

@.formatting.string = private constant [4 x i8] c"%d\0A\00"

define i32 @main() {
entry0:
  %arg = alloca i32
  store i32 12, i32* %arg
  %tmp3 = add i32 12, 22
  %tmp4 = add i32 %tmp3, 12
  %tmp5 = add i32 %tmp3, 12
  ret i32 10
}

declare i32 @printf(i8*, ...)

declare i8* @malloc(i32)
