(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun vv1 () Real)
(declare-fun vv3 () Real)
(declare-fun vv2 () Real)
(declare-fun gamma0 () Real)
(declare-fun theta () Real)
(declare-fun delta () Real)
(declare-fun lambda1 () Real)
(declare-fun mu () Real)
(declare-fun alpha () Real)
(assert (and (< (* (- 1) vv1) 0) (< (* (- 1) vv3) 0) (< (* (- 1) vv2) 0) (= (* (+ (+ (* vv1 (- 1)) (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (* vv1 (- 1))))))))) gamma0) theta) 0) (< (* (- 1) theta) 0) (< (* (- 1) theta (+ (+ 1 (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 vv3)))))) (* lambda1 (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (* vv1 6)))))))) delta) 0) (< (* (- 1) delta) 0) (=  (+ (+ (* vv2 (- 1)) (* lambda1 vv1)) (* mu gamma0)) 0) (=  (+ (* delta (+ (* vv3 (- 1)) vv2)) (* alpha (+ (+ (* vv1 (- 6)) (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (* vv1 (- 6))))))))) (* gamma0 6)))) 0) (=  (+ (+ (+ (* theta (+ (+ 1 (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 vv3)))))) (* theta (+ 1 (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (+ 2 (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 vv3)))))))))))))))) (* delta (+ (+ 1 (* theta (+ (+ 2 (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 2))))))) (* theta (+ 1 (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (+ 2 (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 vv3))))))))))))))))) (* delta (+ 1 (* theta (+ 1 (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 vv3)))))))))))) (* lambda1 (* delta (* theta (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (* vv1 (- 6))))))))))) (* alpha (+ (+ (+ (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (* vv1 36)))))) (* theta (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (+ (* vv1 72) (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (* vv1 72))))))))))))))) (* delta (+ (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (* vv1 72)))))) (* theta (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (+ (* vv1 36) (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (* vv1 36))))))))))))))))) (* alpha (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (* vv3 (* vv1 (* vv1 1296)))))))))))))))) 0) (< (* (- 1) gamma0) 0) (< (* (- 1) mu) 0) (< (* (- 1) alpha) 0)))
(check-sat)
