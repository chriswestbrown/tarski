(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoY () Real)
(declare-fun skoZ () Real)
(assert (>  (+ (+ (+ (- 50) (* skoX (+ 91 (* skoX (- 50))))) (* skoY (+ (+ 91 (* skoX (- 50))) (* skoY (- 50))))) (* skoZ (+ (+ 91 (* skoX (- 50))) (* skoY (+ (+ (- 50) (* skoX (+ (- 91) (* skoX 50)))) (* skoY (* skoX 50))))))) 0))
(check-sat)
