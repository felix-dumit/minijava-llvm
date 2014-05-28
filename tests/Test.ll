@.formatting.string = private constant [4 x i8] c"%d\0A\00"
define i32 @main() {
entry0:

  %a.addr = alloca i32 ;1
  %b.addr = alloca i32 ;2
  %c.addr = alloca i32 ;3


  store i32 0, i32 * %a.addr ;4 ;a = 0

   br label %then ;5
 then:
  %a = load i32 * %a.addr ;6
  %b = load i32 * %b.addr ;7
  %c = load i32 * %c.addr ;8
  
  %tmp0 = add i32 %a, 1 ;9  ;B = A + 1
  store i32 %tmp0, i32 * %b.addr ;10
  
  %tmp6 = add i32 %c, %b ;11  ; C = C + B
  store i32 %tmp6, i32 * %c.addr ;12

  %tmp2 = mul i32 %b, 2 ;13 ; A = B * 2
  store i32 %tmp2, i32 * %a.addr ;14

  %tmp1 = icmp slt i32 %a, 10 ;15 ; A<N
  br i1 %tmp1, label %then, label %else ;16
 else:

  ret i32 %c ;17
}

declare i32 @printf (i8 *, ...)
declare i8 * @malloc (i32)
