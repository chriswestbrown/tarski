(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun x6 () Real)
(declare-fun x13 () Real)
(declare-fun x3 () Real)
(declare-fun x10 () Real)
(declare-fun x0 () Real)
(declare-fun x7 () Real)
(declare-fun x14 () Real)
(declare-fun x4 () Real)
(declare-fun x11 () Real)
(declare-fun x1 () Real)
(declare-fun x8 () Real)
(declare-fun x5 () Real)
(declare-fun x12 () Real)
(declare-fun x2 () Real)
(declare-fun x9 () Real)
(assert (and (>=  x6 0) (>=  x13 0) (>=  x3 0) (>=  x10 0) (>=  x0 0) (>=  x7 0) (>=  x14 0) (>=  x4 0) (>=  x11 0) (>=  x1 0) (>=  x8 0) (>=  x5 0) (>=  x12 0) (>=  x2 0) (>=  x9 0) (>  (+ (+ (+ (* x1 (+ (+ x3 (* x10 x6)) (* x14 (* x10 x6)))) (* x12 (* x5 (* x1 x10)))) (* x2 (+ (+ x4 (* x8 (+ x10 (* x14 x10)))) (* x12 (* x7 x10))))) (* x9 (+ (+ (* x1 (* x13 x6)) (* x5 (* x1 (+ 1 x11)))) (* x2 (+ (+ x7 (* x11 x7)) (* x8 x13)))))) 0) (>=  (+ (+ (+ (* x1 (+ (+ x3 (* x10 x6)) (* x14 (* x10 x6)))) (* x12 (* x5 (* x1 x10)))) (* x2 (+ (+ x4 (* x8 (+ x10 (* x14 x10)))) (* x12 (* x7 x10))))) (* x9 (+ (+ (* x1 (* x13 x6)) (* x5 (* x1 (+ 1 x11)))) (* x2 (+ (+ x7 (* x11 x7)) (* x8 x13)))))) 0) (>=  (+ (+ (+ (* x1 (+ (+ (- 1) (* x14 (* x13 x6))) (* x11 (* x13 x6)))) (* x5 (* x1 (* x11 x11)))) (* x12 (* x5 (* x1 x13)))) (* x2 (+ (+ (* x11 (* x11 x7)) (* x8 (+ (* x14 x13) (* x11 x13)))) (* x12 (* x7 x13))))) 0) (>=  (+ (+ (* x1 (* x14 (* x14 x6))) (* x12 (+ (* x1 (* x13 x6)) (* x5 (* x1 (+ x14 x11)))))) (* x2 (+ (+ (- 1) (* x8 (* x14 x14))) (* x12 (+ (+ (* x14 x7) (* x11 x7)) (* x8 x13)))))) 0) (>  (+ (+ (+ (* x1 (+ (* x10 x6) (* x14 (* x10 x6)))) (* x12 (* x5 (* x1 x10)))) (* x2 (+ (* x8 (+ x10 (* x14 x10))) (* x12 (* x7 x10))))) (* x9 (+ (+ (* x1 (* x13 x6)) (* x5 (* x1 (+ 1 x11)))) (* x2 (+ (+ x7 (* x11 x7)) (* x8 x13)))))) 0) (>=  (+ (+ (+ (* x1 (+ (* x10 x6) (* x14 (* x10 x6)))) (* x12 (* x5 (* x1 x10)))) (* x2 (+ (* x8 (+ x10 (* x14 x10))) (* x12 (* x7 x10))))) (* x9 (+ (+ (* x1 (* x13 x6)) (* x5 (* x1 (+ 1 x11)))) (* x2 (+ (+ x7 (* x11 x7)) (* x8 x13)))))) 0) (>=  (+ (+ (+ (* x1 (+ (* x14 (* x13 x6)) (* x11 (* x13 x6)))) (* x5 (* x1 (+ (- 1) (* x11 x11))))) (* x12 (* x5 (* x1 x13)))) (* x2 (+ (+ (+ (* x7 (- 1)) (* x11 (* x11 x7))) (* x8 (+ (* x14 x13) (* x11 x13)))) (* x12 (* x7 x13))))) 0) (>=  (+ (+ (* x1 (+ (* x6 (- 1)) (* x14 (* x14 x6)))) (* x12 (+ (* x1 (* x13 x6)) (* x5 (* x1 (+ x14 x11)))))) (* x2 (+ (* x8 (+ (- 1) (* x14 x14))) (* x12 (+ (+ (* x14 x7) (* x11 x7)) (* x8 x13)))))) 0) (>  (+ (+ (+ (+ (* x1 (+ (+ (* x10 x6) (* x14 (* x10 x6))) (* x4 (* x6 (- 1))))) (* x5 (* x1 (* x3 (- 1))))) (* x12 (* x5 (* x1 x10)))) (* x2 (+ (+ (* x7 (* x3 (- 1))) (* x8 (+ (+ x10 (* x14 x10)) (* x4 (- 1))))) (* x12 (* x7 x10))))) (* x9 (+ (+ (* x1 (* x13 x6)) (* x5 (* x1 (+ 1 x11)))) (* x2 (+ (+ x7 (* x11 x7)) (* x8 x13)))))) 0) (>=  (+ (+ (+ (+ (* x1 (+ (+ (* x10 x6) (* x14 (* x10 x6))) (* x4 (* x6 (- 1))))) (* x5 (* x1 (* x3 (- 1))))) (* x12 (* x5 (* x1 x10)))) (* x2 (+ (+ (* x7 (* x3 (- 1))) (* x8 (+ (+ x10 (* x14 x10)) (* x4 (- 1))))) (* x12 (* x7 x10))))) (* x9 (+ (+ (* x1 (* x13 x6)) (* x5 (* x1 (+ 1 x11)))) (* x2 (+ (+ x7 (* x11 x7)) (* x8 x13)))))) 0) (>=  (+ (+ (+ (* x1 (+ (+ (* x7 (* x6 (- 1))) (* x14 (* x13 x6))) (* x11 (* x13 x6)))) (* x5 (+ (* x1 (* x11 x11)) (* x5 (* x1 (- 1)))))) (* x12 (* x5 (* x1 x13)))) (* x2 (+ (+ (+ (* x11 (* x11 x7)) (* x8 (+ (+ (* x7 (- 1)) (* x14 x13)) (* x11 x13)))) (* x5 (* x7 (- 1)))) (* x12 (* x7 x13))))) 0) (>=  (+ (+ (+ (+ (* x1 (* x14 (* x14 x6))) (* x8 (* x1 (* x6 (- 1))))) (* x5 (* x1 (* x6 (- 1))))) (* x12 (+ (* x1 (* x13 x6)) (* x5 (* x1 (+ x14 x11)))))) (* x2 (+ (+ (* x7 (* x6 (- 1))) (* x8 (+ (* x14 x14) (* x8 (- 1))))) (* x12 (+ (+ (* x14 x7) (* x11 x7)) (* x8 x13)))))) 0) (> (* (- 1) (+ (+ (+ (+ (+ (+ (* x3 (- 1)) (* x4 (* x6 (- 1)))) (* x11 (* x11 (* x11 (+ (+ x3 (* x7 (* x3 x6))) (* x4 x6)))))) (* x8 (+ (+ (* x10 (* x6 (- 1))) (* x14 (* x10 (* x6 (- 1))))) (* x11 (* x11 (* x11 (* x4 x6))))))) (* x5 (+ (+ (+ (+ (* x3 (- 1)) (* x10 (* x6 (- 1)))) (* x14 (* x10 (* x6 (- 1))))) (* x11 (* x11 (* x11 (+ x3 (* x4 x6)))))) (* x5 (* x11 (* x11 (* x11 x3))))))) (* x12 (+ (+ (+ (+ (+ (+ (+ x10 (* x7 (* x10 (* x6 (- 1))))) (* x14 (+ (+ (+ (* x3 x13) x10) (* x7 (* x3 (* x13 x6)))) (* x14 (* x7 x3))))) (* x4 (* x14 (+ (* x13 x6) (* x14 (+ 1 (* x7 x6))))))) (* x11 (+ (+ (+ (+ (+ (* x3 (* x13 2)) x10) (* x7 (* x3 (* x13 (* x6 2))))) (* x14 (* x7 x3))) (* x4 (+ (* x13 (* x6 2)) (* x14 (+ 1 (* x7 x6)))))) (* x11 (+ (* x7 x3) (* x4 (+ 1 (* x7 x6)))))))) (* x8 (+ (+ (+ (* x14 (* x14 (* x7 x3))) (* x4 (* x14 (+ (* x13 x6) x14)))) (* x11 (+ (+ (* x14 (* x7 x3)) (* x4 (+ (* x13 (* x6 2)) x14))) (* x11 (+ (* x7 x3) x4))))) (* x8 (+ (* x4 (* x14 x14)) (* x11 (+ (* x4 x14) (* x11 x4)))))))) (* x5 (+ (+ (+ (* x14 (+ (* x3 x13) (* x14 (* x7 x3)))) (* x4 (* x14 (* x13 x6)))) (* x11 (+ (+ (+ (* x3 (* x13 2)) (* x14 (* x7 x3))) (* x4 (* x13 (* x6 2)))) (* x11 (* x7 x3))))) (* x5 (+ (+ (* x10 (- 1)) (* x14 (* x3 x13))) (* x11 (* x3 (* x13 2)))))))) (* x12 (+ (+ (+ (* x7 (* x3 x13)) (* x4 (+ x13 (* x7 (* x13 x6))))) (* x8 (+ (+ (* x7 (* x3 x13)) (* x4 x13)) (* x8 (* x4 x13))))) (* x5 (* x7 (* x3 x13)))))))) (* x9 (+ (+ (+ (+ (+ 1 (* x7 (* x6 (- 1)))) (* x11 (+ (+ 1 (* x7 (* x6 (- 1)))) x11))) (* x8 (* x13 (* x6 (- 1))))) (* x5 (+ (* x13 (* x6 (- 1))) (* x5 (+ (- 1) (* x11 (- 1))))))) (* x12 x13))))) 0) (>= (* (- 1) (+ (+ (+ (+ (+ (+ (* x3 (- 1)) (* x4 (* x6 (- 1)))) (* x11 (* x11 (* x11 (+ (+ x3 (* x7 (* x3 x6))) (* x4 x6)))))) (* x8 (+ (+ (* x10 (* x6 (- 1))) (* x14 (* x10 (* x6 (- 1))))) (* x11 (* x11 (* x11 (* x4 x6))))))) (* x5 (+ (+ (+ (+ (* x3 (- 1)) (* x10 (* x6 (- 1)))) (* x14 (* x10 (* x6 (- 1))))) (* x11 (* x11 (* x11 (+ x3 (* x4 x6)))))) (* x5 (* x11 (* x11 (* x11 x3))))))) (* x12 (+ (+ (+ (+ (+ (+ (+ x10 (* x7 (* x10 (* x6 (- 1))))) (* x14 (+ (+ (+ (* x3 x13) x10) (* x7 (* x3 (* x13 x6)))) (* x14 (* x7 x3))))) (* x4 (* x14 (+ (* x13 x6) (* x14 (+ 1 (* x7 x6))))))) (* x11 (+ (+ (+ (+ (+ (* x3 (* x13 2)) x10) (* x7 (* x3 (* x13 (* x6 2))))) (* x14 (* x7 x3))) (* x4 (+ (* x13 (* x6 2)) (* x14 (+ 1 (* x7 x6)))))) (* x11 (+ (* x7 x3) (* x4 (+ 1 (* x7 x6)))))))) (* x8 (+ (+ (+ (* x14 (* x14 (* x7 x3))) (* x4 (* x14 (+ (* x13 x6) x14)))) (* x11 (+ (+ (* x14 (* x7 x3)) (* x4 (+ (* x13 (* x6 2)) x14))) (* x11 (+ (* x7 x3) x4))))) (* x8 (+ (* x4 (* x14 x14)) (* x11 (+ (* x4 x14) (* x11 x4)))))))) (* x5 (+ (+ (+ (* x14 (+ (* x3 x13) (* x14 (* x7 x3)))) (* x4 (* x14 (* x13 x6)))) (* x11 (+ (+ (+ (* x3 (* x13 2)) (* x14 (* x7 x3))) (* x4 (* x13 (* x6 2)))) (* x11 (* x7 x3))))) (* x5 (+ (+ (* x10 (- 1)) (* x14 (* x3 x13))) (* x11 (* x3 (* x13 2)))))))) (* x12 (+ (+ (+ (* x7 (* x3 x13)) (* x4 (+ x13 (* x7 (* x13 x6))))) (* x8 (+ (+ (* x7 (* x3 x13)) (* x4 x13)) (* x8 (* x4 x13))))) (* x5 (* x7 (* x3 x13)))))))) (* x9 (+ (+ (+ (+ (+ 1 (* x7 (* x6 (- 1)))) (* x11 (+ (+ 1 (* x7 (* x6 (- 1)))) x11))) (* x8 (* x13 (* x6 (- 1))))) (* x5 (+ (* x13 (* x6 (- 1))) (* x5 (+ (- 1) (* x11 (- 1))))))) (* x12 x13))))) 0) (>=  (+ (+ (+ (+ (+ (+ (+ (+ (* x10 (- 1)) (* x7 (+ x3 (* x10 x6)))) (* x14 (+ (+ (* x10 (- 1)) (* x7 (* x10 x6))) (* x14 (+ (+ (+ (* x3 (* x13 (- 1))) (* x10 (- 1))) (* x7 (* x3 (* x13 (* x6 (- 1)))))) (* x14 (* x7 (* x3 (- 1))))))))) (* x4 (+ 1 (* x14 (* x14 (+ (* x13 (* x6 (- 1))) (* x14 (+ (- 1) (* x7 (* x6 (- 1))))))))))) (* x11 (+ (+ (* x14 (+ (* x3 (* x13 (- 1))) (* x7 (* x3 (* x13 (* x6 (- 1))))))) (* x4 (* x14 (* x13 (* x6 (- 1)))))) (* x11 (+ (+ (* x3 (* x13 (- 1))) (* x7 (* x3 (* x13 (* x6 (- 1)))))) (* x4 (* x13 (* x6 (- 1))))))))) (* x8 (+ (+ (+ (* x14 (* x14 (* x14 (* x7 (* x3 (- 1)))))) (* x4 (+ 1 (* x14 (* x14 (+ (* x13 (* x6 (- 1))) (* x14 (- 1)))))))) (* x11 (+ (* x4 (* x14 (* x13 (* x6 (- 1))))) (* x11 (* x4 (* x13 (* x6 (- 1)))))))) (* x8 (+ (+ x10 (* x14 x10)) (* x4 (* x14 (* x14 (* x14 (- 1)))))))))) (* x5 (+ (+ (+ (* x14 (* x14 (+ (* x3 (* x13 (- 1))) (* x14 (* x7 (* x3 (- 1))))))) (* x4 (* x14 (* x14 (* x13 (* x6 (- 1))))))) (* x11 (+ (+ (* x14 (* x3 (* x13 (- 1)))) (* x4 (* x14 (* x13 (* x6 (- 1)))))) (* x11 (+ (* x3 (* x13 (- 1))) (* x4 (* x13 (* x6 (- 1))))))))) (* x5 (+ (* x14 (* x14 (* x3 (* x13 (- 1))))) (* x11 (+ (* x14 (* x3 (* x13 (- 1)))) (* x11 (* x3 (* x13 (- 1))))))))))) (* x12 (+ (+ (+ (+ (+ (+ (+ (* x3 (* x13 (* x13 (- 1)))) (* x10 (* x13 (- 1)))) (* x7 (* x3 (* x13 (* x13 (* x6 (- 1))))))) (* x14 (* x7 (* x3 (* x13 (- 2)))))) (* x4 (+ (* x13 (* x13 (* x6 (- 1)))) (* x14 (+ (* x13 (- 2)) (* x7 (* x13 (* x6 (- 2))))))))) (* x11 (+ (* x7 (* x3 (* x13 (- 1)))) (* x4 (+ (* x13 (- 1)) (* x7 (* x13 (* x6 (- 1))))))))) (* x8 (+ (+ (+ (+ (* x7 x10) (* x14 (* x7 (* x3 (* x13 (- 2)))))) (* x4 (+ (* x13 (* x13 (* x6 (- 1)))) (* x14 (* x13 (- 2)))))) (* x11 (+ (* x7 (* x3 (* x13 (- 1)))) (* x4 (* x13 (- 1)))))) (* x8 (+ (* x4 (* x14 (* x13 (- 2)))) (* x11 (* x4 (* x13 (- 1))))))))) (* x5 (+ (+ (+ (+ (+ (* x3 (* x13 (* x13 (- 1)))) (* x7 x10)) (* x14 (* x7 (* x3 (* x13 (- 2)))))) (* x4 (* x13 (* x13 (* x6 (- 1)))))) (* x11 (* x7 (* x3 (* x13 (- 1)))))) (* x5 (* x3 (* x13 (* x13 (- 1)))))))))) (* x9 (+ (+ (+ (+ (+ (* x13 (- 1)) (* x7 (* x13 x6))) (* x14 (* x13 (- 1)))) (* x11 (* x13 (- 1)))) (* x8 (+ (+ x7 (* x11 x7)) (* x8 x13)))) (* x5 (+ x7 (* x11 x7)))))) 0) (>= (* (- 1) (+ (+ (+ (* x11 (* x11 (* x7 (* x6 (- 1))))) (* x8 (+ (* x14 (* x13 (* x6 (- 1)))) (* x11 (+ (* x13 (* x6 (- 1))) (* x11 (* x11 (* x7 x6)))))))) (* x5 (+ (+ (* x14 (* x13 (* x6 (- 1)))) (* x11 (+ (* x13 (* x6 (- 1))) (* x11 (* x11 (* x7 (* x6 2))))))) (* x5 (+ (* x11 (* x11 (- 1))) (* x5 (* x11 (* x11 x11)))))))) (* x12 (+ (+ (+ (+ (+ (* x7 (* x13 (* x6 (- 1)))) (* x14 (* x14 (* x7 (* x7 x6))))) (* x11 (+ (* x14 (* x7 (* x7 x6))) (* x11 (* x7 (* x7 x6)))))) (* x8 (+ (+ (* x14 (* x7 (* x13 x6))) (* x11 (* x7 (* x13 (* x6 2))))) (* x8 (+ (* x14 (* x14 x7)) (* x11 (+ (* x14 x7) (* x11 x7)))))))) (* x5 (+ (+ (+ (* x14 (* x7 (* x13 (* x6 2)))) (* x11 (* x7 (* x13 (* x6 4))))) (* x8 (+ (* x14 (* x14 x7)) (* x11 (+ (* x14 x7) (* x11 x7)))))) (* x5 (+ (+ (+ (* x13 (- 1)) (* x14 (* x14 x7))) (* x11 (+ (* x14 x7) (* x11 x7)))) (* x5 (+ (* x14 x13) (* x11 (* x13 2))))))))) (* x12 (+ (+ (* x7 (* x7 (* x13 x6))) (* x8 (* x8 (* x7 x13)))) (* x5 (+ (* x8 (* x7 x13)) (* x5 (* x7 x13)))))))))) 0) (>= (* (- 1) (+ (+ (+ (* x11 (* x11 (* x11 (* x7 (* x6 x6))))) (* x8 (+ (* x14 (* x14 (* x6 (- 1)))) (* x8 (* x11 (* x11 (* x11 x6))))))) (* x5 (+ (+ (* x14 (* x14 (* x6 (- 1)))) (* x8 (* x11 (* x11 (* x11 x6))))) (* x5 (* x11 (* x11 (* x11 x6))))))) (* x12 (+ (+ (+ (+ (* x14 (* x7 (+ (* x6 (- 1)) (* x13 (* x6 x6))))) (* x11 (* x7 (+ (* x6 (- 1)) (* x13 (* x6 (* x6 2))))))) (* x8 (+ (+ (+ (* x13 (* x6 (- 1))) (* x14 (* x14 (* x7 (* x6 2))))) (* x11 (+ (* x14 (* x7 (* x6 2))) (* x11 (* x7 (* x6 2)))))) (* x8 (+ (+ (* x14 (* x13 x6)) (* x11 (* x13 (* x6 2)))) (* x8 (+ (* x14 x14) (* x11 (+ x14 x11))))))))) (* x5 (+ (+ (+ (+ (* x13 (* x6 (- 1))) (* x14 (* x14 (* x7 x6)))) (* x11 (+ (* x14 (* x7 x6)) (* x11 (* x7 x6))))) (* x8 (+ (* x14 (* x13 x6)) (* x11 (* x13 (* x6 2)))))) (* x5 (+ (* x14 (+ (- 1) (* x13 x6))) (* x11 (+ (- 1) (* x13 (* x6 2))))))))) (* x12 (+ (* x8 (+ (* x7 (* x13 (* x6 2))) (* x8 (* x8 x13)))) (* x5 (* x7 (* x13 x6))))))))) 0) (>= (* (- 1) (+ (+ (+ (+ (* x14 (+ (* x7 (* x13 (* x6 (- 1)))) (* x14 (* x14 (* x7 (* x7 x6)))))) (* x11 (* x7 (* x13 (* x6 (- 1)))))) (* x8 (+ (+ (* x14 (* x14 (* x7 (* x13 x6)))) (* x11 (+ (* x14 (* x7 (* x13 x6))) (* x11 (* x7 (+ (- 1) (* x13 x6))))))) (* x8 (+ (* x14 (+ (* x13 (- 1)) (* x14 (* x14 x7)))) (* x11 (* x13 (- 1)))))))) (* x5 (+ (+ (+ (* x14 (* x14 (* x7 (* x13 (* x6 2))))) (* x11 (+ (* x14 (* x7 (* x13 (* x6 2)))) (* x11 (* x7 (+ (- 1) (* x13 (* x6 2)))))))) (* x8 (* x14 (* x14 (* x14 x7))))) (* x5 (+ (* x14 (* x14 (* x14 x7))) (* x5 (+ (* x14 (* x14 x13)) (* x11 (+ (* x14 x13) (* x11 x13)))))))))) (* x12 (+ (+ (+ (* x14 (* x7 (* x7 (* x13 (* x6 2))))) (* x11 (* x7 (* x7 (* x13 x6))))) (* x8 (+ (* x7 (* x13 (+ (- 1) (* x13 x6)))) (* x8 (+ (* x14 (* x7 (* x13 2))) (* x11 (* x7 x13))))))) (* x5 (+ (+ (* x7 (* x13 (+ (- 1) (* x13 (* x6 2))))) (* x8 (+ (* x14 (* x7 (* x13 2))) (* x11 (* x7 x13))))) (* x5 (+ (+ (* x14 (* x7 (* x13 2))) (* x11 (* x7 x13))) (* x5 (* x13 x13)))))))))) 0) (>= (* (- 1) (+ (+ (+ (+ (* x14 (* x14 (* x7 (+ (* x6 (- 1)) (* x13 (* x6 x6)))))) (* x11 (+ (* x14 (* x7 (* x13 (* x6 x6)))) (* x11 (* x7 (* x13 (* x6 x6))))))) (* x8 (+ (* x14 (* x14 (* x14 (* x7 (* x6 2))))) (* x8 (+ (+ (* x14 (* x14 (+ (- 1) (* x13 x6)))) (* x11 (+ (* x14 (* x13 x6)) (* x11 (* x13 x6))))) (* x8 (* x14 (* x14 x14)))))))) (* x5 (+ (+ (* x14 (* x14 (* x14 (* x7 x6)))) (* x8 (+ (* x14 (* x14 (* x13 x6))) (* x11 (+ (* x14 (* x13 x6)) (* x11 (* x13 x6))))))) (* x5 (+ (* x14 (* x14 (* x13 x6))) (* x11 (+ (* x14 (* x13 x6)) (* x11 (* x13 x6))))))))) (* x12 (+ (+ (* x7 (* x13 (+ (* x6 (- 1)) (* x13 (* x6 x6))))) (* x8 (+ (+ (* x14 (* x7 (+ (- 1) (* x13 (* x6 4))))) (* x11 (* x7 (+ (- 1) (* x13 (* x6 2)))))) (* x8 (+ (* x13 (+ (- 1) (* x13 x6))) (* x8 (+ (* x14 (* x13 2)) (* x11 x13)))))))) (* x5 (+ (+ (+ (* x14 (* x7 (+ (- 1) (* x13 (* x6 2))))) (* x11 (* x7 (+ (- 1) (* x13 x6))))) (* x8 (* x13 (* x13 x6)))) (* x5 (* x13 (* x13 x6))))))))) 0)))
(check-sat)