(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoX () Real)
(declare-fun skoC () Real)
(declare-fun skoS () Real)
(assert (and (> (* (+ 3072000000000 (* skoX (+ 8832000000 (* skoX (+ 12696000 (* skoX 12167)))))) (+ 3072000000000 (* skoX (+ 8832000000 (* skoX (+ 12696000 (* skoX 12167))))))) 0) (> (* (- 1) (+ (- 48000000000000) (* skoX (+ 664800000000 (* skoX (+ (- 4603740000) (* skoX 21253933)))))) (+ (- 48000000000000) (* skoX (+ 664800000000 (* skoX (+ (- 4603740000) (* skoX 21253933)))))) (+ (* skoC (- 86400000)) (* skoS 2025130727))) 0)))
(check-sat)