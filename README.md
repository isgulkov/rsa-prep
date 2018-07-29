
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

1. key generation algorithm:
2. encyption and decryption primitives;
3. an encryption scheme: `RSAES-OAEP` or `RSAES-PKCS-v1_5` (if the first one's too complex);
4. compatibility with the [two ways of private exponent calculation](https://en.wikipedia.org/wiki/RSA_(cryptosystem)#cite_ref-rsa_2-2);
5. [Chinese remainder-based](https://en.wikipedia.org/wiki/RSA_(cryptosystem)#Using_the_Chinese_remainder_algorithm) decryption optimization.

##### Message exchange

1. Three data formats — for cyphertexts, public and private keys:
   1. text form — PGP uses base64 with `+`, `/` and `=`;
   2. whitespace insignificant;
   3. include insignificant labels (just like `--------BEGIN PGP PUBLIC KEY----------`) that will act as "telomeres" — would be able lose or gain a couple characters at the ends for the case of a sloppy selection.
2. CLI application:
   1. facilities for import & export of the above;
   2. *keygen*, *encrypt* and *decrypt* commands.

#### Beyond scope

Might eventually get around to some of these.

1. Both encryption schemes: `RSAES-OAEP` and `RSAES-PKCS-v1_5`;
2. One of the hashes well-supported by RSA signature software (SHA-1 or SHA-256);
3. Signature schemes: `RSASSA-PSS`, `RSASSA-PKCS1-v1_5`;

##### Compatibility with real-world applications

1. GPG (OpenPGP standard):
   1. importing keys from it;
   2. export of keys for it;
   3. signatures interoperability;
   4. encryption interoperability;
2. SSL/TLS:
   1. reading X.509 certificates;
   2. interoperability with OpenSSL (`.pem`, `.der` and shit);
   3. key exchange (?).

##### Security features

1. Secure memory:
   1. compiler-proof memory erasure (at least in destructors);
   2. protection against swaps;
   3. guarded heap allocations (through a C++11-style custom allocator);
2.  Sourcing additional entropy from user environment.

### Goals

TODO

## Building

TODO

## Usage

TODO

<hr />

## Module descriptions

### `intbig_t` — arbitrary-precision integer

The number is internally represented with two data members:

```cpp
    bool is_neg = false;
    std::vector<uint64_t> chunks;
```

- `is_neg` — the number's sign: `true` for negative numbers, `false` otherwise;

- `chunks` — the number's absolute value:

  - the value is stored base 64, the digits are in little-endian (ordered from least to most significant):

    $|x| = chunks[0] + 2^{64} \times chunks[1] + (2^{64})^2 \times chunks[2] + ...$;

  - leading zeroes are not allowed, so the current number's most significant digit can always be accessed as `chunks.back()` or `chunks[chunks.size() - 1]`.

Note that only one representation of zero, — `{ .is_neg=false, chunks={} }`, — is enforced. All other possibilities are illegal (i.e. invalid states of the `intbig_t` objects).

> *Note*: the originally planned representation was GMP-inspired one:
>
> - digits stored a manually-reallocated array;
> - the number's signed represented by the sign of its size field.
>
> Both of these have proven to be huge headaches.

> **TODO**: Find the value for passing into `chunks.reserve()` that will allow 4096-bit modular operations with no further reallocations.
>
> *Note*: the fact that `vector` never automatically shrinks is actually pretty sweet here;

> **TODO**: Benchmark `std::vector<uint64_t>` to determine whether `emplace_back` that everyone (including `clang-Tidy`) is talking about is actually measurably slower than `push_back`. Remember to measure cases where each is called with:
>
> - an integer literal (in a loop);
> - a `uint64_t` local variable whose value is always the same;
> - a `uint64_t` local variable whose value is new each time.

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

##### Division, modulo, power

The arbitrary-precision versions of these operations aren't used by corresponding modular algorithms; thus, these operations are skipped for now.

Though basic impletementations of division and modulo are currently provided for the needs of `to_string` method, they are extremely wasteful and shouldn't be relied on.

#### Todo

Operators and other functions until `intbig_t` considered somewhat finished:

- [x] `intbig_t()` *(zero)*;

- [x] `intbig_t(int64_t)`;

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

- [ ] convert from decimal representation: `intbig_t(std::string)`;

- [ ] binary representation conversions:

  - [ ] from:

    - `static from_bytes(std::string bytes)`;
    - `static from_bytes(std::istream stream)` (reads until EOF);

  - [ ] to:

    - `std::string to_bytes()`;
    -  `void to_bytes(std::ostream stream)`;

    these are roughly the `I2OSP` and `OSP2I` from the [PKCS#1](https://tools.ietf.org/html/rfc3447#page-9) standard;

    the `I2OSP`'s parameter `xLen`, which, to my understanding, is included for overflow protection, is replaced by the following method:

  - [ ] `size_t num_bytes() const` — exactly how many bytes will the previous two methods produce;

    **Note**: consider copying 8 bytes at a time with `reinterpret_cast<uint64_t*>` (check if `std::strings` are contiguous, though);

- [ ] `-` *(unary)*;

- [ ] `++`, `--` *(post)*;

- [ ] `+`, `-`, `*`;

- [ ] set up any implicit "from" conversions for things like `big_x *= "1000000000"` and `big_x = 11`;

- [ ] define conversions to integers — keep them explicit, though, or it will all be pain;

- [ ] decide (not necessarily document) which [named requirements](https://en.cppreference.com/w/cpp/named_req) does and should it implement.

<sup>1</sup> — when will I finally start getting this overload right without StackOverflow?

```cpp
// .h
class MyClass {
    // ...
    friend std::ostream& operator<<(std::ostream& os, const MyClass& value)
    // ...
}

// .cpp
std::ostream& operator<<(std::ostream& os, const MyClass& value) {
	return os << /* ... */ value.x /* ... */;
}
```

#### Possible optimizations

##### Forward an allocator to the underlying `std::vector`

A custom allocator may be used to fulfill one of two goals:

- speed things up, depending on how many temporaries do we end up creating (e.g. monotonic);
- allocate (and deallocate!) with various security considerations, like the big boys in crypto do.

In either case, the `intbig_t` code will need only tiny changes, and code using its default version — no changes:

```cpp
template<typename Alloc=std::allocator<uint64_t>>
class intbig_t_alloc
{
    // ... the old intbig_t code
	std::vector<uint64_t, Alloc> data;
    // ... the old intbig_t code
};

// ...

typedef intbig_t_alloc<> intbig_t;
// This wouldn't be possible:
// typedef intbig_t<> intbig_t;
```

So don't do it early!

##### Small number optimization

In general, an SSO<sup>1</sup>-like optimization, i.e. if numbers $< 2^{64}$ didn't have a vector and were stored completely on the stack, seems to apply perfectly here due to the "law of small numbers"<sup>2</sup>.

For working with 1024- or 4096-bit numbers, though, this is questionable. Will the allocation and locality speedup make up for the requirement to store half a kilobyte or more on the stack?

Moreover, the modular multiplication algorithm will probably both require a temporary and operate in a number longer than the operands.

<sup>1</sup> — *short string optimization*; [here](https://www.youtube.com/watch?v=kPR8h4-qZdk) is a great CppCon talk related to it (never mind the guy's wierd tone and body language — you'd look the same if you took as much Aderall as he does).

<sup>2</sup> — the observations that in most programs, numbers are small: like, 90+% are $< 100$, about 20% are zero — who knows where I heard that, but seems reasonable.

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

   2. [PKCS #1: RSA Cryptography Specifications Version 2.1](https://tools.ietf.org/html/rfc3447) *(the latest verson 2.2 only expands the list of hashes with SHA-256 and others)*;

   3. [RSA — Wikipedia](https://en.wikipedia.org/wiki/RSA_(cryptosystem));

   4. [RSA and Primality Testing](https://imada.sdu.dk/~joan/projects/RSA.pdf) (slides);

   5. [Handbook of Applied Cryptography](http://cacr.uwaterloo.ca/hac/);

      [Chapter 4: Public-Key Parameters](http://cacr.uwaterloo.ca/hac/about/chap4.pdf) *(it's totally somewhere up there with more precise references)*;

   6. [Fast Implementations of RSA Cryptography](https://www.di.ens.fr/~jv/HomePage/pdf/rsa.pdf) *(the dates aren't too good on this, but the presentation is promising)*;

   7. [Anatomy of a GPG Key](https://davesteele.github.io/gpg/2014/09/20/anatomy-of-a-gpg-key/);

   8. [Montgomery modular multiplication](https://en.wikipedia.org/wiki/Montgomery_modular_multiplication) — explicitly advertised for RSA right there;

   9. Validation:

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

   10. Real implementations:

      1. [OpenSSL](https://wiki.openssl.org/index.php/Main_Page);

         OpenSSL's [Command Line Utilities](https://wiki.openssl.org/index.php/Command_Line_Utilities#rsa_.2F_genrsa);

         Description of [OpenSSL-related file formats](https://serverfault.com/a/9717);

      2. [Crypto++](https://www.cryptopp.com/) *(by Wai Dai the Bitcoin guy)*;

         1. [`integer.h`](https://github.com/weidai11/cryptopp/blob/master/integer.h), [`integer.cpp`](https://github.com/weidai11/cryptopp/blob/master/integer.cpp) — their big integer, probably?

            > Wei's original code was much simpler ...
            >

         2. [`modarith.h`](https://github.com/weidai11/cryptopp/blob/master/modarith.h) — modular arithmetic, incl. Montgomery representation;

         3. [`algebra.h`](https://github.com/weidai11/cryptopp/blob/master/algebra.h), [`algebra.cpp`](https://github.com/weidai11/cryptopp/blob/master/algebra.cpp) (its dependency) — some other mathematics;

         4. [`secblock.h`](https://github.com/weidai11/cryptopp/blob/master/secblock.h) — secure memory allocations ([this document](https://download.libsodium.org/doc/helpers/memory_management.html) from other library may provide insight into what's going on there); 

      3. [Python-RSA](https://stuvel.eu/rsa) *(not that anyone right in their mind would actually use it)*;

         [GitHub repo](https://github.com/sybrenstuvel/python-rsa), [PyPI page](https://pypi.org/project/rsa/) (LOL @ project description);

         **May be up for a pull request after I'm finished with this!**

         > Implementation based on the book Algorithm Design by Michael T. Goodrich and Roberto Tamassia, 2002.

         > Running doctests 1000x or until failure

         Found the users:

         > This software was originally written by Sybren Stüvel, Marloes de Boer, Ivo Tamboer and subsequenty improved by Barry Mead, Yesudeep Mangalapilly, and others.

4. Other related algorithms:

   1. Cormen et el.:

      1. 32: Number-Theoretic Algorithms, p. 926;
      2. 31.8: Primality testing *(incl. Miller-Rabin primality test)*, pp. 971–975;

   2. St. Denis — BigNum Math: Implementing Cryptographic Multiple Precision Arithmetic,

      a book by the developer of [LibTomMath](https://www.libtom.net/LibTomMath/) and [LibTomCrypt](https://github.com/libtom/libtomcrypt) that are both written in C and the former reminds me a lot of GMP:

      1. [TeX source](https://github.com/libtom/libtommath/blob/develop/doc/tommath.src);
      2. [Generated PDF](https://github.com/libtom/libtommath/blob/432e3bd8eb40c4e5a40b688da6764d418b1804b2/tommath.pdf) (last version before deletion);
      3. pirated Amazon PDF looks the best, though;

   3. SHA-256:

      1. [A JS implementation](https://www.movable-type.co.uk/scripts/sha256.html) with the "educational use" disclaimer;

   4. `std::vector` replacements:

      1. Some jerk's version: [blog post](https://www.movable-type.co.uk/scripts/sha256.html), [GitHub repo](https://github.com/dendibakh/prep/blob/master/SmallVector.cpp);

      2. [llvm/ADT/SmallVector.h](http://llvm.org/docs/ProgrammersManual.html#llvm-adt-smallvector-h) *(benchmark against this, maybe?)*;

      3. [`folly::fbvector`](https://github.com/facebook/folly/blob/master/folly/docs/FBVector.md) (Facebook/Alexandrescu);

         [FBVectorBenchmark.cpp](https://github.com/facebook/folly/blob/master/folly/test/FBVectorBenchmark.cpp) — benchmarks against several others;

      4. [nsTArray.h](https://dxr.mozilla.org/mozilla-beta/source/xpcom/ds/nsTArray.h) (Mozilla);

      5. [`lni::vector`](https://github.com/lnishan/vector) (some fuck);

      6. [`pector`](https://github.com/aguinet/pector) (some other fuck);

      7. [StackOverflow answer](https://stackoverflow.com/a/2443195): "just use `std::basic_string`!";

         (if I'm replacing `vector`, it doesn't neccessarily need to be nearly as complex as some of these: only a few methods needed, only one use-case considered);

         (some of them boast improving the `memmove` assumptions which are suboptimal in `std::vector` — does this apply to `uint64_t`, though?);

5. [Source File Organization for C++ Projects Part 1: Headers and Sources](https://arne-mertz.de/2016/06/organizing-headers-and-sources/);

   [Source File Organization for C++ Projects Part 2: Directories and Namespaces](https://arne-mertz.de/2016/06/organizing-directories-namespaces/);

6. [C++ Dos and Don'ts](https://www.chromium.org/developers/coding-style/cpp-dos-and-donts) (Chromium);

7. GMP user manual:

   1. [Build Options ](https://gmplib.org/manual/Build-Options.html) — all I had to do was to pass  `--enable-cxx` to `./configure`, it turns out!
   2. [3.1 Headers and Libraries](https://gmplib.org/manual/Headers-and-Libraries.html#Headers-and-Libraries);
   3. [3.4 Conversions](https://ginac.de/CLN/cln.html#Conversions);
   4. [5 Integer functions](https://gmplib.org/manual/Integer-Functions.html#Integer-Functions);
   5. [5.4 Conversion functions](https://gmplib.org/manual/Converting-Integers.html);
   6. [5.12 Input and Output Functions](https://gmplib.org/manual/I_002fO-of-Integers.html);
   7. [12.2 C++ Interface Integers](https://gmplib.org/manual/C_002b_002b-Interface-Integers.html#C_002b_002b-Interface-Integers).

8. Benchmarks:

   1. [Google Benchmark](https://github.com/google/benchmark) (documented mostly in `README.md`);
      1. [`benchmark.h`](https://github.com/google/benchmark/blob/master/include/benchmark/benchmark.h) (includes some additional documentation with examples)
   2. Good-looking error bar plots:
      1. [Styling plots for publication with matplotlib](http://jonchar.net/notebooks/matplotlib-styling/);
      2. [Visualizing Errors](https://jakevdp.github.io/PythonDataScienceHandbook/04.03-errorbars.html) *(uses some "styles" to make it look good)*;
      3. [Matplotlib: beautiful plots with style](http://www.futurile.net/2016/02/27/matplotlib-beautiful-plots-with-style/) *(some kind of explanation of these `maplotlib` styles)*;
   3. [incise.org: Hash Table Benchmarks](http://incise.org/hash-table-benchmarks.html) — doctor, my lines are worrying me 🌝;

9. [Someone really got carried away here](https://github.com/davidcastells/BigInteger) — 10 implementation of the same thing, and he's even some benchmarks against `NTL`.

