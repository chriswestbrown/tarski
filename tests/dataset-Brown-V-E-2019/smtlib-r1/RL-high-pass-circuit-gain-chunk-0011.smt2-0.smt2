(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoC () Real)
(declare-fun skoS () Real)
(assert (and (>  (+ 3 (* skoX (+ 1500 (* skoX 250000)))) 0) (> (* (- 1) (+ (+ (+ 38259 (* skoX (+ 19870500 (* skoX 3188250000)))) (* skoC (+ (- 29250) (* skoX (+ (- 14625000) (* skoX (- 2437500000))))))) (* skoS (+ 4650 (* skoX (+ 2325000 (* skoX 387500000))))))) 0)))
(check-sat)