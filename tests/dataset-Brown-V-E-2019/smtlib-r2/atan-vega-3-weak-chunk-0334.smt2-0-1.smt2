(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(assert (<  (+ (+ (+ 9 (* skoX (+ 36 (* skoX 3)))) (* skoY (+ (* skoX (+ (- 12) (* skoX (- 48)))) (* skoY (+ (+ 6 (* skoX (+ 24 (* skoX 10)))) (* skoY (+ (* skoX (+ (- 4) (* skoX (- 48)))) (* skoY (+ 1 (* skoX (+ (- 12) (* skoX 3)))))))))))) (* skoZ (+ (+ (+ 36 (* skoX 6)) (* skoY (+ (+ 6 (* skoX (+ (- 96) (* skoX (- 6))))) (* skoY (+ (+ (- 12) (* skoX (+ (- 4) (* skoX 60)))) (* skoY (+ (+ 2 (* skoX (* skoX (- 2)))) (* skoY (* skoX (+ (- 2) (* skoX 12))))))))))) (* skoZ (+ 3 (* skoY (+ (+ (- 12) (* skoX (- 6))) (* skoY (+ (+ 1 (* skoX (+ 24 (* skoX 3)))) (* skoY (+ (* skoX (+ (- 2) (* skoX (- 12)))) (* skoY (* skoX skoX))))))))))))) 0))
(check-sat)