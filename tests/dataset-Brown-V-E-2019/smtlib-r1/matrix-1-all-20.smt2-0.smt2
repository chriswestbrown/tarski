(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun x6 () Real)
(declare-fun x13 () Real)
(declare-fun x2 () Real)
(declare-fun x3 () Real)
(declare-fun x4 () Real)
(declare-fun x5 () Real)
(declare-fun x7 () Real)
(declare-fun x10 () Real)
(declare-fun x15 () Real)
(declare-fun x9 () Real)
(declare-fun x0 () Real)
(declare-fun x14 () Real)
(declare-fun x11 () Real)
(declare-fun x1 () Real)
(declare-fun x8 () Real)
(declare-fun x12 () Real)
(assert (and (>=  x6 0) (>=  x13 0) (> (* (- 1) x13 (+ (+ (* x2 (- 1)) x3) (* x4 x2))) 0) (>= (* (- 1) x13 (+ (+ (* x2 (- 1)) x3) (* x4 x2))) 0) (> (* (- 1) x13 (+ (+ (+ (* x3 (- 1)) (* x4 (* x2 (- 1)))) x5) (* x7 x6))) 0) (>= (* (- 1) x13 (+ (+ (+ (* x3 (- 1)) (* x4 (* x2 (- 1)))) x5) (* x7 x6))) 0) (> (* (- 1) x13 (+ (* x2 (+ (- 1) x6)) x5)) 0) (>= (* (- 1) x13 (+ (* x2 (+ (- 1) x6)) x5)) 0) (> (* x13 (+ (+ (+ (* x2 (* x6 (- 1))) x3) (* x5 (- 1))) (* x7 x4))) 0) (>= (* x13 (+ (+ (+ (* x2 (* x6 (- 1))) x3) (* x5 (- 1))) (* x7 x4))) 0) (> (* (- 1) (+ (+ (* x2 (- 1)) x3) (* x4 x2)) x1) 0) (>= (* (- 1) (+ (+ (* x2 (- 1)) x3) (* x4 x2)) x1) 0) (> (* (- 1) (+ (+ (+ (* x3 (- 1)) (* x4 (* x2 (- 1)))) x5) (* x7 x6)) x1) 0) (>= (* (- 1) (+ (+ (+ (* x3 (- 1)) (* x4 (* x2 (- 1)))) x5) (* x7 x6)) x1) 0) (> (* (- 1) (+ (* x2 (+ (- 1) x6)) x5) x1) 0) (>= (* (- 1) (+ (* x2 (+ (- 1) x6)) x5) x1) 0) (> (* (+ (+ (+ (* x2 (* x6 (- 1))) x3) (* x5 (- 1))) (* x7 x4)) x1) 0) (>= (* (+ (+ (+ (* x2 (* x6 (- 1))) x3) (* x5 (- 1))) (* x7 x4)) x1) 0) (>=  x3 0) (>=  x10 0) (> (* x10 x15) 0) (>= (* x10 x15) 0) (> (* x10 x9) 0) (>= (* x10 x9) 0) (>=  x15 0) (>= (* x15 (+ (- 1) x11)) 0) (>=  x9 0) (>= (* x9 (+ (- 1) x11)) 0) (>=  x0 0) (>=  x7 0) (>=  x14 0) (>=  x4 0) (>=  x11 0) (>=  x1 0) (>=  x8 0) (>=  x5 0) (>=  x12 0) (>=  x2 0) (>  (+ (* x3 x13) x12) 0) (>=  (+ (* x3 x13) x12) 0) (>=  (+ (- 1) (* x4 x13)) 0) (>  (+ (* x5 x13) x12) 0) (>=  (+ (* x5 x13) x12) 0) (>=  (+ (- 1) (* x13 x6)) 0)))
(check-sat)
