
<img src="http://www.ahawa.asn.au/images/ahawa---thiihohgec.jpg" align="right" height="100" /> Here I'm slowly chipping away at the goal of implementing the [RSA](https://en.wikipedia.org/wiki/RSA_(cryptosystem)) cryptosystem in C++11 completely from scratch (the only dependency being the standard library, excluding dev ones).

## Motivation

I'm enrolled for course at the uni starting this September where the old man will make us do precisely that, as a year-long project.

Better start early, right? Nothing better to do, anyway — it's not like a have a job.

### Scope

##### Roughly the subproblems of a functional RSA implementation

1. long (*thousands of bits*) integer arithmetics, incl. shifts *(right?)*;

2. $\lambda(n)$ — [Carmichael's totient function](https://en.wikipedia.org/wiki/Carmichael_function),

   $\phi(n)$ — [Euler's totient function](https://en.wikipedia.org/wiki/Euler%27s_totient_function);

3. arithmetics modulo large prime: $+$, $-$, $\times$, $a^b$, $a^{-1}$, $gcd$, $lcm$;

4. cryptographically secure RNG;

5. big integer primality test:

   1. test divisions by a number of small primes (e.g. $< 100$);

   2. multiple [Miller-Rabin tests](https://en.wikipedia.org/wiki/Miller%E2%80%93Rabin_primality_test) with random bases

   3. followed by a single strong<sup>?</sup> [Lucas test](https://en.wikipedia.org/wiki/Lucas_primality_test), using the [FIPS 186-4 tables](http://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.186-4.pdf#page=62) to decide how many to use[<sup>1</sup>](https://crypto.stackexchange.com/a/25881);

      [FIPS 186-3, 5.1 RSA Key Pair Generation](https://csrc.nist.gov/csrc/media/publications/fips/186/3/archive/2009-06-25/documents/fips_186-3.pdf);

      [FIPS 186-4, B.3.2 Generation of Random Primes that are Provably Prime](https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.186-4.pdf#page=61);

      [Handbook of Applied Cryptography, 4. Public-Key Parameters](http://cacr.uwaterloo.ca/hac/about/chap4.pdf):

      - 4.4.1 Random search for probable primes;
      - 4.51 Note *(incremental search)*;
      - 4.4.2 Strong primes

      [OpenSSL: /docs/man1.1.0/crypto/BN_generate_prime](https://www.openssl.org/docs/man1.1.0/crypto/BN_generate_prime.html).

##### The RSA itself

1. the actual algorithms: *keygen*, *encrypt* and *decrypt* functions;
2. padding schemes compatible with some existing implementations;
3. compatibility with the [two ways of private exponent calculation](https://en.wikipedia.org/wiki/RSA_(cryptosystem)#cite_ref-rsa_2-2);
4. [Chinese remainder-based](https://en.wikipedia.org/wiki/RSA_(cryptosystem)#Using_the_Chinese_remainder_algorithm) decryption optimization.

##### Message exchange

1. Three data formats — for cyphertexts, public and private keys:
   1. text form — PGP uses base64 with `+`, `/` and `=`;
   2. whitespace insignificant;
   3. include insignificant labels (just like `--------BEGIN PGP PUBLIC KEY----------`) that will act as "telomeres" — would be able lose or gain a couple characters at the ends for the case of a sloppy selection.
2. CLI application:
   1. facilities for import & export of the above;
   2. *keygen*, *encrypt* and *decrypt* commands.

#### Beyond scope

1. A degree of interoperability with some of the widely-used file formats — `.der`, `.crt`, `.cer`, `.pem`, etc.  (for testing, mainly);

2. Signature algorithm: *sign* and *verify*:

   1. A cryptographic hash (GPG default is SHA-1, but SHA-256 is also well-supported;

      otherwise, the implementation is quite trivial given *encrypt* and *decrypt*;

3. GPG-like keys with fingerprints and self-signed metadata.

##### Security features

1. Secure memory operations (?).

Might still eventually go for some of these, though.

### Goals

TODO

## Building

TODO

## Usage

TODO

<hr />

## Module descriptions

### `intbig_t` — arbitrary-precision integer

The number is stored in a dynamically-allocated array of 32-bit unsigned chunks, with the sign byte as a separate boolean:

```cpp
    size_t len = 0;
    uint32_t* data = nullptr;
```

Here, only the $abs$ of  `len` represents the size of `data`; the sign of `len` corresponds to that of the number. `len == 0` represents zero:

| **represented value **($n$) | `len`                       | `data`                |
| --------------------------- | --------------------------- | --------------------- |
| $n > 0$ (*positive*)        | $\lceil log_{32} n \rceil $ | ` new uint32_t[len]` |
| $n = 0$ *(zero)* | $0$ | `nullptr` |
| $n < 0$ *(negative)* | $-\lceil log_{32} |n| \rceil $ | ` new uint32_t[-len]` |

This clever idea (of reusing `len` for sign, not of maximizing the chunks) has been stolen directly out of [GMP's `mp_z` integer struct](https://gmplib.org/manual/Integer-Internals.html#Integer-Internals):

> **_mp_size**
>
> - The number of limbs, or the negative of that when representing a negative integer. Zero is represented by _mp_size set to zero, in which case the _mp_d data is unused.

> **TODO**: decide what to do when an old number drops to zero (`len = 0;`):
>
> - `delete data; data = nullptr;`,
>
> - or nothing, leaving `data` for future uses?
>
>   With sizes up to some limit?
>

##### Addition

So, addition turned out to be piece of cake, as you can only ever overflow by one bit, and always have somewhere to put it in the next chunk:

$$
{\begin{array}{r}
\quad11111111\\
+11111111\\
+\underline{\quad\quad\quad\,\,\,1}\\
\fbox{1}11111111\\
  \end{array} } \hspace{2em} \longleftarrow  \hspace{2em} {\begin{array}{c}
   \ \ \ \ 11111111\\
+\underline{11111111}\\
\fbox{1}11111110\\
  \end{array} }
$$

The only catch is that sometimes `data` has to be grown.

##### Subtraction

Compared to addition, there are a couple of different cases here concerning carry and whatnot; but, using the corresponding addition opeartor and a unary minus, it is possible to handle all of them in the most pleasant one:

-  $a - b$ where both $a, b$ are positive and $a \ge b$.

| constraints on $a - b$ | what it transforms into    |
| ---------------------- | -------------------------- |
| $a < 0, b\ge 0$        | $-(a + b)$                 |
| $a, b < 0$             | $b - a$  **or** $-(a - b)$ |
| $a \ge 0, b < 0 $      | $a + b$                    |
| $a, b \ge 0, a < b$    | $-(b - a)$                 |

All of these transformations are essentially free; except the case of `a -= b`, where `b` is (for a good reason) `const&`. The comparisons, though, are far from it.

##### Multiplication

It's getting more complicated here. The $O(n^2)$ algorithm where you double the $a$ as go through $b$'s bits is not nearly the optimum; you have to implement either [Karatsuba](https://en.wikipedia.org/wiki/Karatsuba_algorithm) or [Toom–Cook](https://en.wikipedia.org/wiki/Toom%E2%80%93Cook_multiplication), which are on par asymptotically.

There are also FFT-based algorithms that apparently have to be mentioned in every discussion of this topic, but their big-O isn't felt before tens of thousand bytes, so it's doubtful that in our case there are any benefits to them (except, of course, an opportunity to implement the famous FFT that everyone has heard the name of but no one has ever wrote).

> **TODO**: implement some fast multiplication shit

##### Division, modulo

[Fast Division of Large Integers](http://bioinfo.ict.ac.cn/~dbu/AlgorithmCourses/Lectures/Hasselstrom2003.pdf):

- Newton Inversion;
- Barrett's Algorithm.

The modulo is generally a subproduct of division.

##### Power

My current standing on this is the same as on multiplication — repeated squaring is the extent of my knowledge here.





#### Todo

Operators and other functions until `intbig_t` considered ready:

- [x] `intbig_t()` *(zero)*;
- [x] `intbig_t(uint64_t, bool)`, `intbig_t(int64_t)`;
- [x] `==`, `!=`;
- [x] `<`, `<=`, `>=`, `>`;

- [x] implement temporary string dump through ~~`gmpxx`~~ some bigint for the tests;
- [ ] `+=` *(for positives)*;
- [ ] `-=` and `+=` *(for negatives)*;
- [ ] `++`, `--` *(prefix)*;
- [ ] *(copy, move) $\times$ (constructor, assignment) (+ destructor?)*;
- [ ] `size_t size()` — the number's magnitude, i.e. the position of its MSB plus $1$;
- [ ] `<<`, `>>`;
- [ ] `*=`;
- [ ] `%`, `/`, `%=` and `/=` — **for string coversions only**: don't waste time putting any efficient algorithms there — for modulo $p$ the game will most likely be completely different *(don't forget to warn in the doc comments!)*;
- [ ] *convert to string*: `std::to_string($)`, `operator std::string()`, or `operator<<(std::ostream& os, $)`<sup>1</sup>;
  - well, certainly not `std::to_string` — this would be an undefined behavior, as it turns out;
  - `operator std::string()` is not pretty as well — neither implicit nor explicit;
  - guess the way to go is `std::string to_string() const` and the `operator<<`;
- [ ] *convert from string:* `intbig_t(s)`;
- [ ] replace `uint32_t*` member with `std::unique_ptr<uint32_t[]>` *(move constructor needed)*;
- [ ] `-` *(unary)*;
- [ ] `++`, `--` *(post)*;
- [ ] `+`, `-`, `*`.

On next iteration: `+=`, `*=`, `=`, etc. that accept strings, ints and everything — most likely, by defining implicit conversions from them (keep the "to" exclusively explicit, though, or it will all be pain).

<sup>1</sup> — when will I finally start getting this overload right without StackOverflow?

```cpp
std::ostream& operator<<(std::ostream& os, const MyClass& value)
{
	os << /* ... */ value /* ... */;
	return os;
}
```

#### Possible optimizations

##### Use `std::vector<uint32_t>` for `data` instead of hand-`malloc`ed array?

Not only if that's faster (could be — the `vector` probably won't have to do much `new[]`/`delete[]` at all), but even if "not too much slower", certainly switch to that. The mainenability improvement would be dramatic, and I don't really see any performance benefits now compared to that.

Unfortunately, the elegant `int32_t len` would have to step down to boring old `bool is_negative`.

##### 64-bit chunks instead of 32

I've heard about there being no cycle difference neither for add nor multiply; though not sure to which Intel architechtures does that apply.

Anyway, with 64 the multiplication will basically be done the same way, but addition speeds up easily by a factor of two.

> **TODO:** benchmark 32-bit chunks agains 64-bit chunks?
>

##### Stack allocation instead of heap

A heap allocation is hella expensive, and accessing the heap memory through a pointer is detrimental for performance for multiple reasons. That's too bad, as right now every `intbig_t`'s actual bits are always completely allocated on the heap.

Except the one number that is represented as no heap array, zero `len` and null pointer, so, doesn't require any allocation. This representation was chosen for number $0$, as (most probably) the most frequent number in a given program due to the "law of small numbers<sup>1</sup>"

An obvious optimization here would be something like SSO<sup>2</sup> to store the number completely on the stack up to a certain length. This change could've brought significant benefits even if that would just be `8` or `4` bytes.

Unfortunately, the RSA primes our implementation is going to work with are about 1 024 to 16 384 bits long, or 128 to 2048 bytes, which is all kinda on the heavy side for the stack; moreover, the use of move constructors may become about as good as copying with these sizes.

> **TODO**: benchmark 128, 256, 512, 1024 and 2048 byte SNO (*"small" number optimization*) against the baseline and implement that as called for

If the 512 byte stack variable is faster, though, we can basically replace our long integers with fixed-lenth 512B integers, given careful implementation of modular arithmetic afterwards.

> **TODO**: if proves effective, replace the long int with fixed-length int templated on width, e.g.:
>
> ```cpp
> template<size_t LWords>
> class int_fixed
> {
> 	uint64_t data[LWords];
> 	// or uint32_t data[2 * LWords];
> 	//                  ^ wouldn't need constexpr?
> 	/* ... */
> }
> ```

<sup>1</sup> — the observations that in most programs, numbers are small: like, 90+% are $< 100$, about 20% are zero — who knows where I heard that, but seems reasonable.

<sup>2</sup> — *short string optimization*; [here](https://www.youtube.com/watch?v=kPR8h4-qZdk) is a great CppCon talk related to it (never mind the guy's wierd tone and body language — you'd look the same if you took as much Aderall as he does).

**Reserve heap memory ahead of time**

The way everything's done right now, the `data` array is resized one chunk by one, which is unnecessarily expensive.

This calls for a couple possible optimisations *(be careful not to make them prematurely, though)*:

- during one operation is performed, save all the new chunks somewhere on the stack, then reallocate all at once (what operations whould even require large reallocations, though?);
- decouple `len` (the number's parameter) from a new special thing like `__sz` (just the current size of `data`) — so that there's more flexibility overall;
- make so that during the lifetime of an object, `data` just is just more eager to grow than to shrink — hopefully this aren't going to lead to a memory leak or something.

#### Benchmarks

Running benchmarks as soon as the class is implemented.

Planning to benchmark agains the following:

- **high baseline** (something a good real-world application would be likely to use):

  the widely-used [`GMP`](https://gmplib.org/) library, often cited as the fastest out there (their main repo is a mercularial one, without any up-to-date git mirrors, which I solved by just upacking the latest tarball into a folder with the version number and calling int a day);

- **low baseline** (something one of my fellow [Software Engineering](http://www.hse.ru/ba/se) students would be likely to write):

  [a really bad implementation I found on GitHub](https://github.com/panks/BigInteger) that, amazingly, shows up quite high if you google "c++ big integer implementation"<sup>1</sup> and has got a fair bit of likes and retweets, even though it looks like something in competitive programming, and a complete novice at that — e.g. a number is represented as a `std::string` of `[0-9]`;

- [some other, much less awful GitHub repo](https://github.com/calccrypto/integer), though a number is represented as a `std::deque` of bytes — only a step above the previous one;

- [`boost::multiprecision`](https://www.boost.org/doc/libs/1_67_0/libs/multiprecision/doc/html/index.html)'s at the latest tag `v1.67.0` (the builds are a pain, though).

|                                                         | representation                 | ex. Tx. |                             |                                                              |
| ------------------------------------------------------- | ------------------------------ | ------- | --------------------------- | ------------------------------------------------------------ |
| `GMP `                                                  | `uint64_t[]`                   | ✔️       | About the fastest it gets   | `.tar.lz`                                                    |
| `boost::mp`                                             | *varying?*                     | ✔️       | *On its own backend*        | ?                                                            |
| Crypto++                                                | `SecBlock`<sup>2</sup>         | —       |                             | [GitHub](https://github.com/weidai11/cryptopp/blob/master/secblock.h) |
| [`CLN`](https://ginac.de/CLN/cln.html#Modular-integers) | `uint32_t`<sup>3</sup>         | —       |                             | [git]( git://www.ginac.de/cln.git)                           |
| [`NTL`](http://www.shoup.net/ntl/doc/tour-ex1.html)     | ?                              | —       |                             | ?                                                            |
| `intbig_t`                                              | `uint32_t[]`(&rarr; `vector`?) | —       |                             | —                                                            |
| `InfInt`                                                | `vector<int32_t>` base $10^9$  | —       |                             | [GitHub](https://github.com/sercantutar/infint)              |
| `integer`                                               | `deque<uint8_t>`               | —       | Not as bad as the one below | [GitHub](https://github.com/calccrypto/integer/blob/master/integer.h) |
| `BigInteger`                                            | `std::string` (`'0'-'9'`)      | —       | Absolutely hideous          | [GitHub](https://github.com/panks/BigInteger/blob/master/BigInteger.h) |

Also, [`CLN`](https://ginac.de/CLN/cln.html#Modular-integers) seems to have modulars (`ring.h`).

<sup>1</sup> — because in English you're supposed to say "multiple precision", got it.

<sup>2</sup> — "secure memory block" with custom allocators, protecting against side-channels and shit.

<sup>3</sup> — I'm completely lost on why are these called "bytes" or wtf is "`position`":

```cpp
// BYTE-Operationen auf Integers

struct cl_byte {
	uintC size;
	uintC position;
// Konstruktor:
	cl_byte (uintC s, uintC p) : size (s), position (p) {}
};
```

*(make no mistake: `uintC` is a typedef of `uint32_t`)*

<hr />

## Links

1. [Googletest Primer](https://github.com/google/googletest/blob/master/googletest/docs/primer.md);

2. [Длинная арифметика — e-maxx.ru](http://e-maxx.ru/algo/big_integer);

3. RSA:

   1. [R.L. Rivest, A. Shamir, and L. Adleman — A Method for Obtaining Digital Signatures and Public-Key Cryptosystems](http://people.csail.mit.edu/rivest/Rsapaper.pdf);

   2. [RSA — Wikipedia](https://en.wikipedia.org/wiki/RSA_(cryptosystem));

   3. [RSA and Primality Testing](https://imada.sdu.dk/~joan/projects/RSA.pdf) (slides);

   4. [Handbook of Applied Cryptography](http://cacr.uwaterloo.ca/hac/);

      [Chapter 4: Public-Key Parameters](http://cacr.uwaterloo.ca/hac/about/chap4.pdf) *(it's totally somewhere up there with more precise references)*;

   5. [Fast Implementations of RSA Cryptography](https://www.di.ens.fr/~jv/HomePage/pdf/rsa.pdf) *(the dates aren't too good on this, but the presentation is promising)*;

   6. [Anatomy of a GPG Key](https://davesteele.github.io/gpg/2014/09/20/anatomy-of-a-gpg-key/);

   7. [Montgomery modular multiplication](https://en.wikipedia.org/wiki/Montgomery_modular_multiplication) — explicitly advertised for RSA right there;

   8. Validation:

      1. [The 186-4 RSA Validation System (RSA2VS)](https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-Algorithm-Validation-Program/documents/dss2/rsa2vs.pdf);

      2. [Project Wycheproof](https://github.com/google/wycheproof) — some test vectors;

      3. [Some other library's description](https://github.com/pyca/cryptography/blob/master/docs/development/test-vectors.rst) of where they get their test vectors;

      4. [Crypto++'s test vectors](https://github.com/weidai11/cryptopp/tree/master/TestVectors);

      5. [BoringSSL](https://boringssl.googlesource.com/boringssl/) — OpenSSL fork, has [fuzzing provisions](https://boringssl.googlesource.com/boringssl/+/HEAD/FUZZING.md) and [some tests](https://boringssl.googlesource.com/boringssl/+/ce3773f9fe25c3b54390bc51d72572f251c7d7e6/crypto/evp/evp_tests.txt);

      6. Fuzzing:

         1. [CDF – crypto differential fuzzing](https://github.com/kudelskisecurity/cdf) — a Go application;

            [`rsaenc`](https://github.com/kudelskisecurity/cdf#rsaenc-rsa-encryption-oaep-or-pkcs-15) — its interface for RSA encryption/decryption;

         2. llvm's [libFuzzer](http://llvm.org/docs/LibFuzzer.html), [AddressSanitizer](http://clang.llvm.org/docs/AddressSanitizer.html)

         3. [American fuzzy lop](http://lcamtuf.coredump.cx/afl/) (AFL) — *complex file semantics* and shit;

         4. [libFuzzer Tutorial](https://github.com/google/fuzzer-test-suite/blob/master/tutorial/libFuzzerTutorial.md);

         5. [fuzzer-test-suite](https://github.com/google/fuzzer-test-suite);

   9. Real implementations:

      1. [Crypto++](https://www.cryptopp.com/) *(by Wai Dai the Bitcoin guy)*;

         1. [`integer.h`](https://github.com/weidai11/cryptopp/blob/master/integer.h), [`integer.cpp`](https://github.com/weidai11/cryptopp/blob/master/integer.cpp) — their big integer, probably?

            > Wei's original code was much simpler ...
            >
            > ... memory findings ...

         2. [`modarith.h`](https://github.com/weidai11/cryptopp/blob/master/modarith.h) — modular arithmetic, incl. Montgomery representation;

         3. [`algebra.h`](https://github.com/weidai11/cryptopp/blob/master/algebra.h), [`algebra.cpp`](https://github.com/weidai11/cryptopp/blob/master/algebra.cpp) (its dependency) — some other mathematics;

      2. [OpenSSL](https://wiki.openssl.org/index.php/Main_Page);

         OpenSSL's [Command Line Utilities](https://wiki.openssl.org/index.php/Command_Line_Utilities#rsa_.2F_genrsa);

         Description of [OpenSSL-related file formats](https://serverfault.com/a/9717);

4. [Source File Organization for C++ Projects Part 1: Headers and Sources](https://arne-mertz.de/2016/06/organizing-headers-and-sources/);

   [Source File Organization for C++ Projects Part 2: Directories and Namespaces](https://arne-mertz.de/2016/06/organizing-directories-namespaces/);

5. GMP user manual:

   1. [Build Options ](https://gmplib.org/manual/Build-Options.html) — all I had to do was to pass  `--enable-cxx` to `./configure`, it turns out!
   2. [3.1 Headers and Libraries](https://gmplib.org/manual/Headers-and-Libraries.html#Headers-and-Libraries);
   3. [3.4 Conversions](https://ginac.de/CLN/cln.html#Conversions);
   4. [5 Integer functions](https://gmplib.org/manual/Integer-Functions.html#Integer-Functions);
   5. [5.4 Conversion functions](https://gmplib.org/manual/Converting-Integers.html);
   6. [5.12 Input and Output Functions](https://gmplib.org/manual/I_002fO-of-Integers.html);
   7. [12.2 C++ Interface Integers](https://gmplib.org/manual/C_002b_002b-Interface-Integers.html#C_002b_002b-Interface-Integers).

6. Good-looking error bar plots:

   1. [Styling plots for publication with matplotlib](http://jonchar.net/notebooks/matplotlib-styling/);
   2. [Visualizing Errors](https://jakevdp.github.io/PythonDataScienceHandbook/04.03-errorbars.html) *(uses some "styles" to make it look good)*;
   3. [Matplotlib: beautiful plots with style](http://www.futurile.net/2016/02/27/matplotlib-beautiful-plots-with-style/) *(some kind of explanation of these `maplotlib` styles)*;

7. [Someone really got carried away here](https://github.com/davidcastells/BigInteger) — 10 implementation of the same thing, and he's even some benchmarks against `NTL`.

