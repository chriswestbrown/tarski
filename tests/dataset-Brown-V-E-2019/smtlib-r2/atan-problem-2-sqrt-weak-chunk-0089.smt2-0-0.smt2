(set-logic QF_NRA)
(set-info :source | Produced by tarski version 1.27  |)
(set-info :smt-lib-version 2.0)
(declare-fun skoA () Real)
(declare-fun skoB () Real)
(declare-fun skoT () Real)
(assert (>  (+ (* skoB (* skoB (+ (* skoA (* skoA (* skoA 2))) (* skoB (* skoA (* skoA (- 2))))))) (* skoT (* skoT (+ (+ (* skoA (+ (- 1) (* skoA (* skoA 2)))) (* skoB (+ (+ 1 (* skoA (* skoA (- 2)))) (* skoB (+ (* skoA (+ 2 (* skoA (+ (- 2) (* skoA (- 1)))))) (* skoB (+ (- 2) (* skoA skoA)))))))) (* skoT (* skoT (+ (+ (* skoA (+ 2 (* skoA (+ (- 2) (* skoA (- 1)))))) (* skoB (+ (+ (- 2) (* skoA skoA)) (* skoB (+ (+ (- 2) (* skoA (- 1))) skoB))))) (* skoT (* skoT (+ (+ (- 2) (* skoA (- 1))) skoB)))))))))) 0))
(check-sat)