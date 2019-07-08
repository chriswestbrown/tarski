(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun x3 () Real)
(declare-fun x1 () Real)
(declare-fun x5 () Real)
(declare-fun x2 () Real)
(declare-fun x4 () Real)
(declare-fun x0 () Real)
(assert (and (>=  x3 0) (>= (* (- 1) x3 (+ (- 1) x5) x1) 0) (>= (* (- 1) x3 x1 (+ (- 1) (* x5 (* x5 x3)))) 0) (> (* x3 x5 x2) 0) (>= (* x3 x5 x2) 0) (>= (* x3 x5 (+ (- 1) x3)) 0) (> (* (- 1) x3 (+ (* x2 (+ (- 1) (* x5 (+ 1 (* x5 x3))))) (* x4 (+ 1 (* x5 x3))))) 0) (>= (* (- 1) x3 (+ (* x2 (+ (- 1) (* x5 (+ 1 (* x5 x3))))) (* x4 (+ 1 (* x5 x3))))) 0) (>= (* (- 1) x3 x3 (+ (- 1) (* x5 (* x5 x3)))) 0) (>=  x1 0) (> (* x1 x5 x2) 0) (>= (* x1 x5 x2) 0) (>= (* x1 x5 (+ (- 1) x3)) 0) (> (* (- 1) x1 (+ (* x2 (+ (- 1) (* x5 (+ 1 (* x5 x3))))) (* x4 (+ 1 (* x5 x3))))) 0) (>= (* (- 1) x1 (+ (* x2 (+ (- 1) (* x5 (+ 1 (* x5 x3))))) (* x4 (+ 1 (* x5 x3))))) 0) (> (* (- 1) x1 (+ (* x2 (+ (- 1) x5)) x4)) 0) (>= (* (- 1) x1 (+ (* x2 (+ (- 1) x5)) x4)) 0) (>=  x5 0) (>=  x2 0) (>=  x0 0) (>=  x4 0)))
(check-sat)