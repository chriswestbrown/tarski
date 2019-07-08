(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun alpha () Real)
(declare-fun delta () Real)
(declare-fun lambda1 () Real)
(declare-fun theta () Real)
(declare-fun vv1 () Real)
(declare-fun vv3 () Real)
(assert (=  (+ (+ (* delta (+ 1 delta)) (* theta (+ (+ 1 (* delta (+ 2 delta))) (* theta (+ 1 delta))))) (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (+ (* vv1 (+ (+ (* alpha 36) (* delta (* alpha 72))) (* theta (+ (+ (* alpha 72) (* delta (* alpha 36))) (* lambda1 (* delta (- 6))))))) (* vv3 (+ (* theta (+ (+ 1 (* delta (+ 2 delta))) (* theta (+ 2 (* delta 2))))) (* vv3 (* vv3 (* vv3 (* vv3 (+ (* vv1 (* vv1 (* alpha (* alpha 1296)))) (* vv3 (+ (* vv1 (* theta (+ (* alpha 72) (* delta (* alpha 36))))) (* vv3 (* theta (* theta (+ 1 delta)))))))))))))))))))) 0))
(check-sat)