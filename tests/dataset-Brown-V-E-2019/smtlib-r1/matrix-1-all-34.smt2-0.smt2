(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun x6 () Real)
(declare-fun x13 () Real)
(declare-fun x3 () Real)
(declare-fun x10 () Real)
(declare-fun x0 () Real)
(declare-fun x17 () Real)
(declare-fun x7 () Real)
(declare-fun x14 () Real)
(declare-fun x2 () Real)
(declare-fun x4 () Real)
(declare-fun x8 () Real)
(declare-fun x11 () Real)
(declare-fun x1 () Real)
(declare-fun x18 () Real)
(declare-fun x15 () Real)
(declare-fun x5 () Real)
(declare-fun x12 () Real)
(declare-fun x19 () Real)
(declare-fun x9 () Real)
(declare-fun x16 () Real)
(assert (and (>=  x6 0) (>=  x13 0) (>=  x3 0) (>=  x10 0) (>=  x0 0) (>=  x17 0) (>=  x7 0) (>=  x14 0) (> (* (- 1) x14 (+ (+ (+ x7 (* x2 (- 1))) (* x4 (* x3 (- 1)))) (* x8 (+ x2 (* x4 x3))))) 0) (>= (* (- 1) x14 (+ (+ (+ x7 (* x2 (- 1))) (* x4 (* x3 (- 1)))) (* x8 (+ x2 (* x4 x3))))) 0) (> (* (- 1) (+ (+ (+ x7 (* x2 (- 1))) (* x4 (* x3 (- 1)))) (* x8 (+ x2 (* x4 x3)))) x1) 0) (>= (* (- 1) (+ (+ (+ x7 (* x2 (- 1))) (* x4 (* x3 (- 1)))) (* x8 (+ x2 (* x4 x3)))) x1) 0) (>=  x4 0) (>=  x11 0) (>=  x1 0) (>  x18 0) (>=  x18 0) (>=  x8 0) (>=  x15 0) (>=  x5 0) (>=  x12 0) (>=  x2 0) (>=  x19 0) (>=  x9 0) (>=  x16 0) (> (* (- 1) (+ (+ (+ (+ (* x0 (- 1)) (* x2 x6)) (* x4 (* x3 x6))) (* x1 (+ (* x2 (- 1)) (* x4 (* x3 (- 1)))))) x5)) 0) (>= (* (- 1) (+ (+ (+ (+ (* x0 (- 1)) (* x2 x6)) (* x4 (* x3 x6))) (* x1 (+ (* x2 (- 1)) (* x4 (* x3 (- 1)))))) x5)) 0) (>  (+ (+ (* x0 (- 1)) (* x11 x10)) x9) 0) (>=  (+ (+ (* x0 (- 1)) (* x11 x10)) x9) 0) (>=  (+ (* x1 (- 1)) (* x12 x10)) 0) (> (* (- 1) (+ (+ (+ (+ (+ (* x13 (- 1)) (* x4 (* x14 (- 1)))) (* x11 x17)) x15) (* x12 (+ (* x2 x17) (* x4 (* x17 x3))))) (* x16 x4))) 0) (>= (* (- 1) (+ (+ (+ (+ (+ (* x13 (- 1)) (* x4 (* x14 (- 1)))) (* x11 x17)) x15) (* x12 (+ (* x2 x17) (* x4 (* x17 x3))))) (* x16 x4))) 0) (>  (+ x7 (* x8 x2)) 0) (>=  (+ x7 (* x8 x2)) 0) (>=  (+ (- 1) (* x8 x3)) 0) (> (* (- 1) (+ (* x13 (- 1)) x11)) 0) (>= (* (- 1) (+ (* x13 (- 1)) x11)) 0) (>= (* (- 1) (+ (* x14 (- 1)) x12)) 0) (>  (+ (+ (* x13 (- 1)) x18) (* x19 x11)) 0) (>=  (+ (+ (* x13 (- 1)) x18) (* x19 x11)) 0) (>=  (+ (* x14 (- 1)) (* x19 x12)) 0) (>=  (+ (- 1) x19) 0)))
(check-sat)
