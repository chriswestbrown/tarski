(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun x6 () Real)
(declare-fun x13 () Real)
(declare-fun x15 () Real)
(declare-fun x14 () Real)
(declare-fun x16 () Real)
(declare-fun x3 () Real)
(declare-fun x10 () Real)
(declare-fun x8 () Real)
(declare-fun x1 () Real)
(declare-fun x12 () Real)
(declare-fun x0 () Real)
(declare-fun x7 () Real)
(declare-fun x4 () Real)
(declare-fun x11 () Real)
(declare-fun x5 () Real)
(declare-fun x2 () Real)
(declare-fun x9 () Real)
(assert (and (>=  x6 0) (> (* x6 x13) 0) (>= (* x6 x13) 0) (> (* x6 x15) 0) (>= (* x6 x15) 0) (>= (* x6 (+ (- 1) x14)) 0) (>= (* x6 (+ (- 1) x16)) 0) (>=  x13 0) (> (* x13 x3) 0) (>= (* x13 x3) 0) (> (* x13 x10) 0) (>= (* x13 x10) 0) (> (* x13 x8) 0) (>= (* x13 x8) 0) (>=  x15 0) (> (* x15 x3) 0) (>= (* x15 x3) 0) (> (* x15 x10) 0) (>= (* x15 x10) 0) (> (* x15 x8) 0) (>= (* x15 x8) 0) (>= (* (+ (- 1) x14) x3) 0) (>= (* (+ (- 1) x14) x10) 0) (>= (* (+ (- 1) x14) x8) 0) (>= (* (+ (- 1) x16) x3) 0) (>= (* (- 1) (+ (- 1) x16) x3 x14) 0) (>= (* (+ (- 1) x16) x10) 0) (>= (* (+ (- 1) x16) x8) 0) (>=  x3 0) (>= (* x3 (+ (* x1 (* x14 (- 1))) x12)) 0) (>=  x10 0) (>= (* x10 (+ (* x1 (- 1)) x12)) 0) (>= (* (- 1) x10 (+ (* x14 (- 1)) x16)) 0) (>=  x8 0) (>=  x14 0) (>=  x0 0) (>=  x7 0) (>=  x4 0) (>=  x11 0) (>=  x1 0) (>=  x5 0) (>=  x12 0) (> (* x12 x2) 0) (>= (* x12 x2) 0) (>= (* x12 (+ (- 1) x3)) 0) (>=  x2 0) (>=  x9 0) (>=  x16 0) (>  (+ (+ (+ x0 (* x4 (+ (* x3 (* x6 (- 1))) (* x1 (* x3 x3))))) (* x5 (- 1))) (* x2 (+ (* x6 (- 1)) (* x1 (+ 1 x3))))) 0) (>=  (+ (+ (+ x0 (* x4 (+ (* x3 (* x6 (- 1))) (* x1 (* x3 x3))))) (* x5 (- 1))) (* x2 (+ (* x6 (- 1)) (* x1 (+ 1 x3))))) 0) (> (* (- 1) (+ (+ (+ (+ (* x0 (- 1)) x7) (* x4 (+ (* x8 (* x10 x3)) (* x1 (* x3 (* x3 (- 1))))))) (* x2 (+ (* x8 x10) (* x1 (+ (- 1) (* x3 (- 1))))))) (* x9 x8))) 0) (>= (* (- 1) (+ (+ (+ (+ (* x0 (- 1)) x7) (* x4 (+ (* x8 (* x10 x3)) (* x1 (* x3 (* x3 (- 1))))))) (* x2 (+ (* x8 x10) (* x1 (+ (- 1) (* x3 (- 1))))))) (* x9 x8))) 0) (> (* (- 1) (+ (+ (+ (+ (* x0 (- 1)) (* x4 (+ (* x1 (* x3 (* x3 (- 1)))) (* x12 (* x10 (* x3 x3)))))) x11) (* x2 (+ (* x1 (+ (- 1) (* x3 (- 1)))) (* x12 (+ 1 (* x10 x3)))))) (* x9 (* x12 x3)))) 0) (>= (* (- 1) (+ (+ (+ (+ (* x0 (- 1)) (* x4 (+ (* x1 (* x3 (* x3 (- 1)))) (* x12 (* x10 (* x3 x3)))))) x11) (* x2 (+ (* x1 (+ (- 1) (* x3 (- 1)))) (* x12 (+ 1 (* x10 x3)))))) (* x9 (* x12 x3)))) 0) (>  (+ (+ (+ (* x8 (* x13 (- 1))) (* x7 (- 1))) x11) (* x2 x12)) 0) (>=  (+ (+ (+ (* x8 (* x13 (- 1))) (* x7 (- 1))) x11) (* x2 x12)) 0) (>=  (+ (* x8 (* x14 (- 1))) (* x12 x3)) 0) (>  (+ (+ (+ (* x1 (* x3 (* x13 (- 1)))) (* x0 (- 1))) x11) (* x2 (+ (* x1 (- 1)) x12))) 0) (>=  (+ (+ (+ (* x1 (* x3 (* x13 (- 1)))) (* x0 (- 1))) x11) (* x2 (+ (* x1 (- 1)) x12))) 0) (>  (+ (+ (* x0 (- 1)) (* x4 (+ (* x1 (- 1)) x12))) x11) 0) (>=  (+ (+ (* x0 (- 1)) (* x4 (+ (* x1 (- 1)) x12))) x11) 0) (>  (+ (+ (* x0 (- 1)) x11) (* x9 (+ (* x1 (- 1)) x12))) 0) (>=  (+ (+ (* x0 (- 1)) x11) (* x9 (+ (* x1 (- 1)) x12))) 0) (> (* (- 1) (+ (+ (+ (+ x13 (* x15 (- 1))) (* x4 (+ (* x3 (* x3 (* x16 (- 1)))) (* x10 (* x3 (* x3 x14)))))) (* x2 (+ (+ (+ x14 (* x16 (- 1))) (* x3 (* x16 (- 1)))) (* x10 (* x3 x14))))) (* x9 (* x3 x14)))) 0) (>= (* (- 1) (+ (+ (+ (+ x13 (* x15 (- 1))) (* x4 (+ (* x3 (* x3 (* x16 (- 1)))) (* x10 (* x3 (* x3 x14)))))) (* x2 (+ (+ (+ x14 (* x16 (- 1))) (* x3 (* x16 (- 1)))) (* x10 (* x3 x14))))) (* x9 (* x3 x14)))) 0) (> (* (- 1) (+ (+ (+ (* x13 (- 1)) x15) (* x3 (* x16 x13))) (* x2 (+ (* x14 (- 1)) x16)))) 0) (>= (* (- 1) (+ (+ (+ (* x13 (- 1)) x15) (* x3 (* x16 x13))) (* x2 (+ (* x14 (- 1)) x16)))) 0) (> (* (- 1) (+ (+ (* x13 (- 1)) x15) (* x4 (+ (* x14 (- 1)) x16)))) 0) (>= (* (- 1) (+ (+ (* x13 (- 1)) x15) (* x4 (+ (* x14 (- 1)) x16)))) 0) (> (* (- 1) (+ (+ (* x13 (- 1)) x15) (* x9 (+ (* x14 (- 1)) x16)))) 0) (>= (* (- 1) (+ (+ (* x13 (- 1)) x15) (* x9 (+ (* x14 (- 1)) x16)))) 0)))
(check-sat)