(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoY () Real)
(declare-fun skoX () Real)
(declare-fun skoZ () Real)
(assert (and (=  skoY 0) (<  skoX 0) (>  (+ (+ (- 3) (* skoX (+ (* skoY (+ 3 (* skoY (- 12)))) (* skoX (+ (+ (- 1) (* skoY (- 12))) (* skoX (+ (- 4) (* skoY (+ 1 (* skoY (- 4))))))))))) (* skoZ (+ (- 12) (* skoX (+ (* skoY 12) (* skoX (+ (- 4) (* skoX (* skoY 4))))))))) 0) (>  skoZ 0) (>  (+ 1 skoX) 0) (<  (+ (- 1) skoY) 0)))
(check-sat)
