(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(assert (<  (+ (+ (+ 120 (* skoX (* skoX (+ (- 20) (* skoX skoX))))) (* skoY (+ (* skoX (+ (- 40) (* skoX (* skoX 4)))) (* skoY (+ (+ (- 20) (* skoX (* skoX 6))) (* skoY (+ (* skoX 4) skoY))))))) (* skoZ (+ (+ (* skoX (+ (- 40) (* skoX (* skoX 4)))) (* skoY (+ (+ (- 40) (* skoX (* skoX 12))) (* skoY (+ (* skoX 12) (* skoY 4)))))) (* skoZ (+ (+ (+ (- 20) (* skoX (* skoX 6))) (* skoY (+ (* skoX 12) (* skoY 6)))) (* skoZ (+ (+ (* skoX 4) (* skoY 4)) skoZ))))))) 0))
(check-sat)