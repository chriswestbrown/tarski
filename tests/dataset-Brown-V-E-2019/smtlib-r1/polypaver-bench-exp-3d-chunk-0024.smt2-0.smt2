(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(assert (and (> (* (- 1) (+ (+ (+ (- 48) (* skoX (+ 24 (* skoX (+ (- 6) skoX))))) (* skoY (+ (+ 24 (* skoX (+ (- 12) (* skoX 3)))) (* skoY (+ (+ (- 6) (* skoX 3)) skoY))))) (* skoZ (+ (+ (+ 24 (* skoX (+ (- 12) (* skoX 3)))) (* skoY (+ (+ (- 12) (* skoX 6)) (* skoY 3)))) (* skoZ (+ (+ (+ (- 6) (* skoX 3)) (* skoY 3)) skoZ))))) (+ (+ (+ (- 48) (* skoX (+ 24 (* skoX (+ (- 6) skoX))))) (* skoY (+ (+ 24 (* skoX (+ (- 12) (* skoX 3)))) (* skoY (+ (+ (- 6) (* skoX 3)) skoY))))) (* skoZ (+ (+ (+ 24 (* skoX (+ (- 12) (* skoX 3)))) (* skoY (+ (+ (- 12) (* skoX 6)) (* skoY 3)))) (* skoZ (+ (+ (+ (- 6) (* skoX 3)) (* skoY 3)) skoZ)))))) 0) (>  (+ (+ skoX skoY) skoZ) 0)))
(check-sat)