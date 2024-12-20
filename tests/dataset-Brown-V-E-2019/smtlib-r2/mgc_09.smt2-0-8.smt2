(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun alpha () Real)
(declare-fun delta () Real)
(declare-fun gamma0 () Real)
(declare-fun vv1 () Real)
(declare-fun vv2 () Real)
(declare-fun vv3 () Real)
(assert (= (* (- 1) (+ (+ (+ (* gamma0 (* alpha (- 9))) (* vv1 (* alpha 9))) (* vv2 (* delta (- 1)))) (* vv3 (+ delta (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (* vv1 (* alpha 9)))))))))))))) 0))
(check-sat)
