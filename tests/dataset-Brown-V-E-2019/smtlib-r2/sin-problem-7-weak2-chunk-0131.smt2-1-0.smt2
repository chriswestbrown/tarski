(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoA () Real)
(declare-fun skoX () Real)
(assert (>  (+ (* skoA (+ (- 362880) (* skoA (* skoA (+ 60480 (* skoA (* skoA (+ (- 3024) (* skoA (* skoA (+ 72 (* skoA (* skoA (- 1)))))))))))))) (* skoX (+ (* skoA (* skoA (+ (- 120960000) (* skoA (* skoA (+ 6048000 (* skoA (* skoA (+ (- 144000) (* skoA (* skoA 2000))))))))))) (* skoX (* skoX 120960000))))) 0))
(check-sat)
