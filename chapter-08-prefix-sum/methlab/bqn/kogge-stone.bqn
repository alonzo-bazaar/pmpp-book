# in this file I just try and grok the algorithms presented

# these implementations aren't really that performant, and they
# are not meant to be, for an efficient scan in bqn just use
# https://mlochbaum.github.io/BQN/doc/scan.html

# these are my exploration of how in the fuck do these things do
# their thing, in this wonderful data parallel methlab that is bqn

# silly little Kogge Stone
KS ← {s𝕊a: # stride size and arrayy
   na ← s (⊢ + 0⊸»⍟⊣) a    # new array
   ns ← s×2                # new stride

   # uncomment to print intermediate results
   # •Show na

   # questionable tail recursion
   # especially considering the amount of iterations this takes
   # is known in advance, it's just ⌊ 2 ⋆⁼ ≠ a
   # so I could have just used ⍟ like a normal
   # person but nooooo, gotta be a relapsed lisper in fucking bqn
   ns (⊣ ≥ ≠∘⊢)◶⟨𝕊, ⊢⟩ na 
}

# test against bqn builtin +`
! (1 Ks 20⥊1) ≡ (1+↕20)
! (1 Ks 20⥊1) ≡ (+` 20⥊1)
! (1 Ks ↕20) ≡ (+` ↕20)

! (1 Ks 21⥊1) ≡ (1+↕21)
! (1 Ks 21⥊1) ≡ (+` 21⥊1)
! (1 Ks ↕21) ≡ (+` ↕21)

# time against +`
# docs for •_timed may be found at
# https://mlochbaum.github.io/BQN/spec/system.html#time
•Show 1000 1⊸KS •_timed ↕10000 # 0.026184233384 seconds/iteration
•Show 1000 +` •_timed ↕10000 # 4.284432e¯6 seconds/iteration
