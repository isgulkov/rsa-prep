<img src="docs/rsa.jpg" align="right" height="100" /> Here I'm slowly chipping away at the goal of implementing the [RSA](https://en.wikipedia.org/wiki/RSA_(cryptosystem)) cryptosystem in C++11 completely from scratch (the only dependency being the standard library, excluding dev ones).

## Motivation

For my 3rd year at the university, I've enrolled for a course where the old man will make us do precisely that, as a year-long project.

Better start early, right? So I started in July. Nothing better to do, anyway — it's not like I had a job at the time.

### Scope

So, by the deadline in May, I had a mostly-functional implementation of a long integer with proper *64-bit limbs*, which some in the class believed to be impossible, LOL.

This allowed me to quickly cook up correct implementations of the Miller-Rabin test and the RSA itself, including SHA-256-based sign/verify operation.

The thing got me an 8/10. One could see it as a "fuck you", considering 6/10 was the grade you got for shit you literally copied from RosettaCode. But then again, objectively, it's far from finished.

#### TODO

First, the `intbig_t` itself contains some serious flaws:

1. There's only one multiplication algorithm, which for really large numbers can be suboptimal. I really implement Toom-Cook for the larger ones. E.g., here are the GMP thresholds:

    - 30 limbs — 2-way Toom-Cook (i.e. Karatsuba)
    - 100 limbs — 3-way Toom-Cook
    - 300 limbs — 4-way Toom-Cook

   I should benchmark my code to set my own, though.

2. On division/remainder, I just gave up and did it in half-limbs, so it's slow as shit. The proper algorithm was just too hard to follow. Have to give it another try. 

3. There are probably tons of reasonable TODOs in `intbit_t.cpp`. Like, it should totally have a `constexpr` constructor, even if this means moving to C++17 (with the `<filesystem>` and shit).

   Also, the file itself is just huge. There's got to be a way to decompose it.

---

Second, the prime (and hence, key) generation is extremely slow compared to OpenSSL. After studying its labyrinth of a codebase, I've found the modular power used in Miller-Rabin as the main culprit ([here](https://github.com/openssl/openssl/blob/f844f9eb44186df2f8b0cfd3264b4eb003d8c61a/crypto/bn/bn_prime.c#L321-L335)).

   1. It has to be done using the Montgomery setup. As with proper division, I knew about it, but was just not in the mood to do the work. My insistence on complete unit testing has probably contributed to it.
    
      After spending a good 40 hours thoroughly testing the big int, I outright hate unit tests now. I should at least find a more convenient way to put the cases in.
      
      Better check out the references they left in the comments:
      
      ```c
      // http://security.ece.orst.edu/publications.html, e.g.
      // http://security.ece.orst.edu/koc/papers/j37acmon.pdf and
      // sections 3.8 and 4.2 in http://security.ece.orst.edu/koc/papers/r01rsasw.pdf
      ```
    
   2. With the faster Miller-Rabins, they do [a lot more of them](https://github.com/openssl/openssl/blob/f844f9eb44186df2f8b0cfd3264b4eb003d8c61a/crypto/bn/bn_prime.c#L87-L99) for 2^-256 error rate. The 2^-80 numbers I got from the *Handbook of Applied Cryptography* are probably way outdated.
   
   3. They also do [a number of trial divisions](https://github.com/openssl/openssl/blob/f844f9eb44186df2f8b0cfd3264b4eb003d8c61a/crypto/bn/bn_prime.c#L70-L85), with the actual number probably based on benchmarks as well.
   
      Strangely, while they do have the product of a lot of factors, the GCD with it is only used in tests. In the actual thing, they just divide by each one. Got to have fast division for that.
   
   4. Generally, having google benchmark here would be nice.

---

Perhaps most importantly, there's not interop with PGP formats. However tedious, I have to implement the I/O of PGP's PEM-encoded messages and keys. Thus, my app would be able to interoperate with, say, GPG to some extent, which will be a clear demonstration that it actually works!

Decoding the format is just pure byte-fuckery. I'd just give up use a library for this, but sadly, there isn't one.

A bigger problem is the messages GPG produces by default. It compresses the cleartext with "ZIP" (aka DEFLATE), then encrypts it with AES-CBC-something. I mean, SHA-256 was quite easy, and AES, while much more complex, is still doable. But DEFLATE?.. I mean, without it, the "project" wouldn't be complete, but I don't know if my time is as worthless as that. We'll see.s

## Building

It should build like any other CMake project. I haven't had the time to check yet, though.

## Usage

Oh, you don't, you seriously don't.

