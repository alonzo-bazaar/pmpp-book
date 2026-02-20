# improvised bar chart for better debugging of BK
Bar ← { 
    m ← ⌈´𝕩
    > {'#'¨⌾(𝕩⊸↑) m⥊' '}¨ 𝕩
}

# Brent Kung, now even sillier (fucking stupid) than before
# First Step
BKF ← {s𝕊a: # stride size and arrayy
   di ← 1-˜ (0=s⊸|)⊸/1+↕∘≠a  # destination indices
   si ← di - ⌊ s÷2           # source indices

   
   na ← ((si⊸⊏+di⊸⊏) a)⌾(di⊸⊏) a # new array
   ns ← 2×s                      # new stride

   ns (⊣ > ≠∘⊢)◶⟨𝕊, ⊢⟩ na        # same ungodly tail recursion
}

2 BKF 10⥊1 # = ⟨ 1 2 1 4 1 2 1 8 1 2 ⟩
           # you can see how 1, 2, 4, 8 are already what they'll
           # be in the final result
           # now we should get the other ones

2 BKF 16⥊1 # = ⟨ 1 2 1 4 1 2 1 8 1 2 1 4 1 2 1 16 ⟩


# bks, second step
# NOTE:
# the diagram (in third edition at least) shows an
# increasing number of threads for the second part, but the code
# uses a decreasing number of threads for the aphorementioned
# second part
# I shall go with what the code is doing

# SUBSEQUENT NOTE:
# I thought the order of additions didn't matter because of
# commutativitity
# let's just say I was fucking wrong, this shit has side effects, me
# fucking dumbass, it's in place, what the fuck
# it must "start with few threads and end with many threads"
# the order in which intermediate results are distributed throught the
# array, unsuprisingly, fucking matters

BKS ← {s𝕊a: # stride size and arrayy
   # source indices
   # same-ish formula as di in last fun
   # but we skip the last one

   si ← ¯1↓ 1-˜ (0=s⊸|)⊸/1+↕∘≠a
   # destination indices
   # now they are just after source indies, instead of just before
   di ← si + ⌊ s÷2
   
   na ← ((si⊸⊏+di⊸⊏) a)⌾(di⊸⊏) a # new array
   ns ← ⌊s÷2                     # new stride

   # •Show si
   # •Show di
   # •Show na

   ns (1≥⊣)◶⟨𝕊, ⊢⟩ na ;

   # monadic case for default length
   𝕊a:
   # default length is
   # the greatest power of two such that (2×it) ≤ ≠a
   # the ⌾ is overkill but I find it funny
   s ← ⌊⌾(2⊸⋆⁼) 2÷˜ ≠a
   s𝕊a
}

# and putting BKS after BKF is our histogram function
Bar 2 BKF 16⥊1

! (BKS 2 BKF 32⥊1) ≡ (+` 32⥊1)
! (BKS 2 BKF 16⥊1) ≡ (+` 16⥊1)
! (BKS 2 BKF 08⥊1) ≡ (+` 08⥊1)
! (BKS 2 BKF 04⥊1) ≡ (+` 04⥊1)
! (BKS 2 BKF 02⥊1) ≡ (+` 02⥊1) # this one doesn't work! (fix later)
! (BKS 2 BKF 01⥊1) ≡ (+` 01⥊1)

! (BKS 2 BKF ↕32) ≡ (+` ↕32)
! (BKS 2 BKF ↕16) ≡ (+` ↕16)
! (BKS 2 BKF ↕08) ≡ (+` ↕08)
! (BKS 2 BKF ↕04) ≡ (+` ↕04)
! (BKS 2 BKF ↕02) ≡ (+` ↕02)
! (BKS 2 BKF ↕01) ≡ (+` ↕01)

# also, BK as we have it now does not work on non powers of two
# it always starts with one thread only, which, if we truncate
# the diagram at figure 8.4 to only work on 10 elements, is
# not really the way to do it for arbitrarily many thingies
BKS 2 BKF 10⥊1
BKS 2 BKF 24⥊1
BKS 2 BKF 24⥊1
