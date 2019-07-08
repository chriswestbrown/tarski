(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoY () Real)
(declare-fun skoX () Real)
(declare-fun pi () Real)
(assert (and (>  skoY 0) (=  (+ 362880 (* skoY (* skoY (+ (- 60480) (* skoY (* skoY (+ 3024 (* skoY (* skoY (+ (- 72) (* skoY skoY))))))))))) 0) (>  (+ (* skoY (* skoY (+ (- 60480) (* skoY (* skoY (+ 3024 (* skoY (* skoY (+ (- 72) (* skoY skoY)))))))))) (* skoX (* skoX (+ 60480 (* skoX (* skoX (+ (- 3024) (* skoX (* skoX 72))))))))) 0) (>  (+ (* skoY (* skoY (+ (- 840) (* skoY (* skoY 42))))) (* skoX (* skoX (+ 840 (* skoX (* skoX (+ (- 42) (* skoX skoX)))))))) 0) (>  skoX 0) (>  (+ (- 15707963) (* pi 5000000)) 0) (<  (+ (- 31415927) (* pi 10000000)) 0) (>  (+ (* skoY (- 2)) pi) 0) (<  (+ (* skoY (- 1)) skoX) 0)))
(check-sat)