(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun x6 () Real)
(declare-fun x13 () Real)
(declare-fun x3 () Real)
(declare-fun x20 () Real)
(declare-fun x10 () Real)
(declare-fun x0 () Real)
(declare-fun x17 () Real)
(declare-fun x7 () Real)
(declare-fun x14 () Real)
(declare-fun x4 () Real)
(declare-fun x21 () Real)
(declare-fun x11 () Real)
(declare-fun x16 () Real)
(declare-fun x1 () Real)
(declare-fun x12 () Real)
(declare-fun x18 () Real)
(declare-fun x8 () Real)
(declare-fun x15 () Real)
(declare-fun x9 () Real)
(declare-fun x5 () Real)
(declare-fun x22 () Real)
(declare-fun x2 () Real)
(declare-fun x19 () Real)
(assert (and (>=  x6 0) (>=  x13 0) (>=  x3 0) (>=  x20 0) (>=  x10 0) (>=  x0 0) (>=  x17 0) (>=  x7 0) (>=  x14 0) (>=  x4 0) (>=  x21 0) (>=  x11 0) (>= (* x11 (+ (- 1) x16)) 0) (>= (* (+ (- 1) x16) x12) 0) (>= (* (+ (- 1) x16) x8) 0) (>= (* (+ (- 1) x16) x9) 0) (>=  x1 0) (>= (* (- 1) x1 x12) 0) (>=  x12 0) (>= (* (- 1) x12 x18) 0) (>=  x18 0) (>=  x8 0) (>=  x15 0) (> (* x15 (+ x8 x9)) 0) (>= (* x15 (+ x8 x9)) 0) (> (* x15 (+ x11 x12)) 0) (>= (* x15 (+ x11 x12)) 0) (>=  x5 0) (>=  x22 0) (>=  x2 0) (>=  x19 0) (>=  x9 0) (>=  x16 0) (>  (+ (+ x0 (* x5 (- 1))) (* x2 x1)) 0) (>=  (+ (+ x0 (* x5 (- 1))) (* x2 x1)) 0) (>=  (+ (* x6 (- 1)) x3) 0) (>  (+ (+ x0 (* x7 (- 1))) (* x2 x1)) 0) (>=  (+ (+ x0 (* x7 (- 1))) (* x2 x1)) 0) (>= (* (- 1) (+ (* x3 (- 1)) x8)) 0) (>= (* (- 1) (+ (* x4 (- 1)) x9)) 0) (>  (+ (+ (* x3 (* x13 (- 1))) (* x1 (* x10 (- 1)))) (* x2 x1)) 0) (>=  (+ (+ (* x3 (* x13 (- 1))) (* x1 (* x10 (- 1)))) (* x2 x1)) 0) (>= (* (- 1) (+ (+ (* x3 (- 1)) (* x14 x3)) (* x1 x11))) 0) (> (* (- 1) (+ (+ (* x18 x10) (* x2 (* x18 (- 1)))) (* x19 x13))) 0) (>= (* (- 1) (+ (+ (* x18 x10) (* x2 (* x18 (- 1)))) (* x19 x13))) 0) (>= (* (- 1) (+ (* x18 x11) (* x19 (+ (- 1) x14)))) 0) (> (* (- 1) (+ (+ (* x10 (- 1)) (* x11 (* x21 (- 1)))) x22)) 0) (>= (* (- 1) (+ (+ (* x10 (- 1)) (* x11 (* x21 (- 1)))) x22)) 0) (> (* (- 1) (+ (+ (+ (* x10 (- 1)) (* x12 (* x21 (- 1)))) (* x15 (* x11 (- 1)))) x2)) 0) (>= (* (- 1) (+ (+ (+ (* x10 (- 1)) (* x12 (* x21 (- 1)))) (* x15 (* x11 (- 1)))) x2)) 0) (>  (+ x13 (* x21 (+ (- 1) x14))) 0) (>=  (+ x13 (* x21 (+ (- 1) x14))) 0) (>  (+ x13 (* x15 x14)) 0) (>=  (+ x13 (* x15 x14)) 0) (>=  (+ (- 1) (* x16 x14)) 0)))
(check-sat)
