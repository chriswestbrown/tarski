(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoY () Real)
(declare-fun skoX () Real)
(declare-fun skoZ () Real)
(assert (and (>  skoY 0) (=  (+ (+ (+ (- 9) (* skoY (* skoY (+ (- 3) (* skoY 12))))) (* skoX (+ (* skoY 12) (* skoX (+ (+ (- 6) (* skoY (* skoY (+ (- 10) (* skoY 40))))) (* skoX (+ (* skoY (+ 4 (* skoY 32))) (* skoX (+ (- 1) (* skoY (+ 16 (* skoY (+ (- 3) (* skoY 12)))))))))))))) (* skoZ (+ (+ (+ (- 36) (* skoY (+ (- 6) (* skoY 24)))) (* skoX (+ (+ (- 6) (* skoY (+ 120 (* skoY (+ 6 (* skoY (- 24))))))) (* skoX (+ (+ 12 (* skoY (+ 4 (* skoY (- 76))))) (* skoX (+ (+ (- 2) (* skoY (+ 8 (* skoY (+ 2 (* skoY (- 8))))))) (* skoX (* skoY (+ 2 (* skoY (- 20)))))))))))) (* skoZ (+ (+ (- 3) (* skoY 12)) (* skoX (+ (+ 12 (* skoY (+ 6 (* skoY (- 24))))) (* skoX (+ (+ (- 1) (* skoY (+ (- 20) (* skoY (+ (- 3) (* skoY 12)))))) (* skoX (+ (* skoY (+ 2 (* skoY 4))) (* skoX (* skoY (* skoY (+ (- 1) (* skoY 4)))))))))))))))) 0) (<  skoX 0) (=  (+ (+ (* skoY (- 1)) (* skoX (- 1))) (* skoZ (+ (- 1) (* skoX skoY)))) 0) (>  skoZ 0) (>  (+ 1 skoX) 0) (<  (+ (- 1) skoY) 0)))
(check-sat)
