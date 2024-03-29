(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun x6 () Real)
(declare-fun x23 () Real)
(declare-fun x13 () Real)
(declare-fun x30 () Real)
(declare-fun x3 () Real)
(declare-fun x20 () Real)
(declare-fun x10 () Real)
(declare-fun x27 () Real)
(declare-fun x28 () Real)
(declare-fun x5 () Real)
(declare-fun x12 () Real)
(declare-fun x9 () Real)
(declare-fun x0 () Real)
(declare-fun x17 () Real)
(declare-fun x34 () Real)
(declare-fun x7 () Real)
(declare-fun x24 () Real)
(declare-fun x14 () Real)
(declare-fun x31 () Real)
(declare-fun x4 () Real)
(declare-fun x21 () Real)
(declare-fun x11 () Real)
(declare-fun x1 () Real)
(declare-fun x18 () Real)
(declare-fun x35 () Real)
(declare-fun x8 () Real)
(declare-fun x25 () Real)
(declare-fun x15 () Real)
(declare-fun x32 () Real)
(declare-fun x22 () Real)
(declare-fun x29 () Real)
(declare-fun x2 () Real)
(declare-fun x19 () Real)
(declare-fun x26 () Real)
(declare-fun x16 () Real)
(declare-fun x33 () Real)
(assert (and (>=  x6 0) (>=  x23 0) (>=  x13 0) (>=  x30 0) (>=  x3 0) (>=  x20 0) (>=  x10 0) (>=  x27 0) (> (* x27 (+ 1 x28) x5) 0) (>= (* x27 (+ 1 x28) x5) 0) (> (* x27 (+ 1 x28) x12) 0) (>= (* x27 (+ 1 x28) x12) 0) (> (* x27 (+ 1 x28) x9) 0) (>= (* x27 (+ 1 x28) x9) 0) (>= (* (+ 1 x28) x5 (+ (- 1) x28)) 0) (>= (* (+ 1 x28) x12 (+ (- 1) x28)) 0) (>= (* (+ 1 x28) x9 (+ (- 1) x28)) 0) (>=  x5 0) (>=  x12 0) (>=  x9 0) (>=  x0 0) (>=  x17 0) (>=  x34 0) (>=  x7 0) (>=  x24 0) (>=  x14 0) (>= (* x14 (+ (- 1) x28)) 0) (>= (* (+ (- 1) x28) x21 x28) 0) (>= (* (+ (- 1) x28) x18) 0) (>=  x31 0) (>=  x4 0) (>=  x21 0) (>=  x28 0) (>=  x11 0) (>=  x1 0) (>=  x18 0) (>=  x35 0) (>=  x8 0) (>=  x25 0) (>=  x15 0) (>=  x32 0) (>=  x22 0) (>=  x29 0) (>=  x2 0) (>=  x19 0) (>=  x26 0) (>=  x16 0) (>=  x33 0) (>  (+ (+ x0 (* x4 (- 1))) (* x2 x1)) 0) (>=  (+ (+ x0 (* x4 (- 1))) (* x2 x1)) 0) (>= (* (- 1) (+ (* x3 (- 1)) x5)) 0) (>  (+ (+ (+ (* x6 (- 1)) x0) (* x8 (* x7 (- 1)))) (* x2 x1)) 0) (>=  (+ (+ (+ (* x6 (- 1)) x0) (* x8 (* x7 (- 1)))) (* x2 x1)) 0) (>= (* (- 1) (+ (+ (* x3 (- 1)) x10) (* x7 x9))) 0) (>  (+ (+ x6 (* x11 (- 1))) (* x2 x7)) 0) (>=  (+ (+ x6 (* x11 (- 1))) (* x2 x7)) 0) (>= (* (- 1) (+ (* x10 (- 1)) x12)) 0) (> (* (- 1) (+ (+ (+ (* x6 (- 1)) x13) (* x2 (* x7 (- 1)))) (* x16 x15))) 0) (>= (* (- 1) (+ (+ (+ (* x6 (- 1)) x13) (* x2 (* x7 (- 1)))) (* x16 x15))) 0) (>= (* (- 1) (+ (* x10 (- 1)) x14)) 0) (> (* (- 1) (+ (+ (+ (+ (+ (* x6 (- 1)) (* x20 x3)) x0) (* x1 x17)) (* x2 (* x7 (- 1)))) (* x16 (* x19 x1)))) 0) (>= (* (- 1) (+ (+ (+ (+ (+ (* x6 (- 1)) (* x20 x3)) x0) (* x1 x17)) (* x2 (* x7 (- 1)))) (* x16 (* x19 x1)))) 0) (>= (* (- 1) (+ (+ (* x10 (- 1)) (* x21 x3)) (* x18 x1))) 0) (>  (+ (+ x6 (* x23 (- 1))) (* x22 x7)) 0) (>=  (+ (+ x6 (* x23 (- 1))) (* x22 x7)) 0) (>= (* (- 1) (+ (* x10 (- 1)) x24)) 0) (> (* (- 1) (+ (+ (+ (* x6 (- 1)) x13) (* x22 (* x7 (- 1)))) (* x16 x15))) 0) (>= (* (- 1) (+ (+ (+ (* x6 (- 1)) x13) (* x22 (* x7 (- 1)))) (* x16 x15))) 0) (> (* (- 1) (+ (+ (+ (+ (+ (* x6 (- 1)) x0) (* x1 x17)) (* x25 x3)) (* x22 (* x7 (- 1)))) (* x16 (* x19 x1)))) 0) (>= (* (- 1) (+ (+ (+ (+ (+ (* x6 (- 1)) x0) (* x1 x17)) (* x25 x3)) (* x22 (* x7 (- 1)))) (* x16 (* x19 x1)))) 0) (>= (* (- 1) (+ (+ (* x10 (- 1)) (* x18 x1)) (* x26 x3))) 0) (>  (+ (+ (* x14 x27) (* x15 x27)) (* x29 (* x15 (+ (- 1) x28)))) 0) (>=  (+ (+ (* x14 x27) (* x15 x27)) (* x29 (* x15 (+ (- 1) x28)))) 0) (> (* (- 1) (+ (+ (+ (* x30 (- 1)) (* x8 x34)) (* x2 (* x31 (- 1)))) x33)) 0) (>= (* (- 1) (+ (+ (+ (* x30 (- 1)) (* x8 x34)) (* x2 (* x31 (- 1)))) x33)) 0) (>=  (+ (+ (* x34 (* x9 (- 1))) (* x35 (- 1))) x32) 0) (>  (+ (+ (+ (+ (+ (* x30 (- 1)) (* x31 (* x17 (- 1)))) (* x32 (* x20 (- 1)))) (* x2 x34)) (* x16 (* x19 (* x31 (- 1))))) x33) 0) (>=  (+ (+ (+ (+ (+ (* x30 (- 1)) (* x31 (* x17 (- 1)))) (* x32 (* x20 (- 1)))) (* x2 x34)) (* x16 (* x19 (* x31 (- 1))))) x33) 0) (>= (* (- 1) (+ (+ (* x18 x31) (* x35 (- 1))) (* x32 x21))) 0) (>  (+ (+ (+ (+ (+ (* x30 (- 1)) (* x31 (* x17 (- 1)))) (* x32 (* x25 (- 1)))) (* x22 x34)) (* x16 (* x19 (* x31 (- 1))))) x33) 0) (>=  (+ (+ (+ (+ (+ (* x30 (- 1)) (* x31 (* x17 (- 1)))) (* x32 (* x25 (- 1)))) (* x22 x34)) (* x16 (* x19 (* x31 (- 1))))) x33) 0) (>= (* (- 1) (+ (+ (* x18 x31) (* x35 (- 1))) (* x26 x32))) 0) (>  (+ (+ x17 (* x22 (- 1))) (* x16 (+ x18 x19))) 0) (>=  (+ (+ x17 (* x22 (- 1))) (* x16 (+ x18 x19))) 0) (>  (+ (+ (+ x17 (* x2 (- 1))) (* x19 x27)) (* x16 x18)) 0) (>=  (+ (+ (+ x17 (* x2 (- 1))) (* x19 x27)) (* x16 x18)) 0) (>  (+ (+ (+ x17 (* x18 x27)) (* x2 (- 1))) (* x16 x19)) 0) (>=  (+ (+ (+ x17 (* x18 x27)) (* x2 (- 1))) (* x16 x19)) 0) (>  (+ (* x18 x27) (* x19 (+ x27 (* x29 (+ (- 1) x28))))) 0) (>=  (+ (* x18 x27) (* x19 (+ x27 (* x29 (+ (- 1) x28))))) 0) (>  (+ (+ x8 (* x2 (- 1))) (* x16 x9)) 0) (>=  (+ (+ x8 (* x2 (- 1))) (* x16 x9)) 0) (>  (+ (+ (+ (* x9 x27) x8) (* x22 (- 1))) (* x16 (* x9 x28))) 0) (>=  (+ (+ (+ (* x9 x27) x8) (* x22 (- 1))) (* x16 (* x9 x28))) 0) (>  (+ x20 (* x16 (+ (- 1) x21))) 0) (>=  (+ x20 (* x16 (+ (- 1) x21))) 0) (>  (+ (+ x20 (* x21 x27)) (* x16 (+ (- 1) (* x21 x28)))) 0) (>=  (+ (+ x20 (* x21 x27)) (* x16 (+ (- 1) (* x21 x28)))) 0) (>  (+ (+ (+ x20 (* x27 (- 1))) (* x28 (* x20 (- 1)))) (* x21 (+ x27 (* x28 x27)))) 0) (>=  (+ (+ (+ x20 (* x27 (- 1))) (* x28 (* x20 (- 1)))) (* x21 (+ x27 (* x28 x27)))) 0) (>  (+ x25 (* x16 (+ (- 1) x26))) 0) (>=  (+ x25 (* x16 (+ (- 1) x26))) 0) (>  (+ x25 (* x26 x27)) 0) (>=  (+ x25 (* x26 x27)) 0) (>=  (+ (- 1) (* x26 x28)) 0)))
(check-sat)
