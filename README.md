
<img src="http://www.ahawa.asn.au/images/ahawa---thiihohgec.jpg" align="right" height="48" /> Here I'm slowly chipping away at the goal of implementing the [RSA](https://en.wikipedia.org/wiki/RSA_(cryptosystem)) cryptosystem in C++11 completely from scratch (the only dependency being the standard library, excluding dev ones).

## Motivation

I'm enrolled for a course starting this September where the old man will make us do precisely that, as a year-long project.

Better start early, right? Nothing better to do, anyway — it's not like a have a job.

### Scope

##### Roughly the subproblems of a functional RSA implementation

1. long (*thousands of bits*) integer arithmetics, incl. shifts *(right?)*;

2. $\lambda(n)$ — [Carmichael's totient function](https://en.wikipedia.org/wiki/Carmichael_function),

   $\phi(n)$ — [Euler's totient function](https://en.wikipedia.org/wiki/Euler%27s_totient_function);

3. long integer arithmetics modulo large prime: $+$, $-$, $\times$, $a^b$, $a^{-1}$, $gcd$, $lcm$;

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

1. the algorithm itself: *keygen*, *encrypt* and *decrypt* functions;
2. padding schemes compatible with some existing implementations;
3. compatibility with the [two ways of private exponent calculation](https://en.wikipedia.org/wiki/RSA_(cryptosystem)#cite_ref-rsa_2-2);
4. [Chinese remainder-based](https://en.wikipedia.org/wiki/RSA_(cryptosystem)#Using_the_Chinese_remainder_algorithm) decryption optimization.

##### Message exchange

1. Three data formats — for cyphertexts, public and private keys:
   1. text form, all printable, easily selectable, e.g. Bitcoin's base58;
   2. whitespace insignificant;
   3. have "telomeres" — can lose or gain some characters at the ends in case of a sloppy selection;
   4. include insignificant labels (just like `--------BEGIN PGP PUBLIC KEY----------`);
2. CLI application:
   1. facilities for import & export of the above;
   2. *keygen*, *encrypt* and *decrypt* commands.

#### Beyond scope

1. Key fingerprints;
2. message and metadata authentication.

### Goals

TODO

## Building

TODO

## Usage

TODO

<hr />

## Module description

### `uint_long` — long integer

> **TODO**: what the fuck? rename into `int_long` 🌝

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

This clever idea has been stolen directly out of [GMP's `mp_z` integer struct](https://gmplib.org/manual/Integer-Internals.html#Integer-Internals):

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

#### Todo

Operators and other functions until `uint_long` considered ready:

- [x] `uint_long()` *(zero)*;
- [x] `uint_long(uint64_t, bool)`, `uint_long(int64_t)`;
- [x] `==`, `!=`;
- [x] `<`, `<=`, `>=`, `>`;
- [ ] `-` *(unary)*;
- [ ] `++`, `--` *(prefix)*;
- [ ] `+=`, `-=`;
- [ ] `*=`;
- [ ] `%=`, `/=`;
- [ ] *convert to string*: `std::to_string($)`, `operator std::string()`, or `operator<<(std::ostream& os, $)`;
- [ ] *convert from string:* `uint_long(s)`;
- [ ] *(copy, move) $\times$ (constructor, assignment) (+ destructor?)*;
- [ ] replace `uint32_t*` member with `std::unique_ptr<uint32_t[]>` *(move constructor needed)*;
- [ ] `++`, `--` *(post)*;
- [ ] `+`, `-`, `*`, `%`, `/`.

#### Possible optimizations

##### 64-bit chunks instead of 32

I've heard about there being no cycle difference neither for add nor multiply; though not sure to which Intel architechtures does that apply.

Anyway, with 64 you'll need fewer of both additions and multiplications, *but* then carry will be a separate calculation for add or three for multiply... Anyway, this needs investigation.

> **TODO:** benchmark 32-bit chunks agains 64-bit chunks
> 
> **TODO:** if that doesn't work out, try calculating in clang's 128-bit ints *(that's only good on x86-64, right?)*

##### Stack allocation instead of heap

A heap allocation is hella expensive, and accessing the heap memory through a pointer is detrimental for performance for multiple reasons. That's too bad, as right now every `unit_long`'s actual bits are always completely allocated on the heap.

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

#### Benchmarks

Running benchmarks as soon as the class is implemented.

Planning to benchmark agains the following:

- **high baseline** (something a good real-world application would be likely to use):

  the widely-used [`GMP`](https://gmplib.org/) library, often cited as the fastest out there

  *(too bad its official repo is [a mercurial one](https://gmplib.org/repo/gmp), though, with no current git mirror);

- **low baseline** (something one of my fellow [Software Engineering](http://www.hse.ru/ba/se) students would be likely to write):

  [a really bad implementation I found on GitHub](https://github.com/panks/BigInteger) that, amazingly, shows up quite high if you google "c++ big integer implementation" and has got a fair bit of likes and retweets, even though it looks like something in competitive programming, and a complete novice at that — e.g. a number is represented as a `std::string` of `[0-9]`;

- [some other, much less awful GitHub repo](https://github.com/calccrypto/integer), though a number is represented as a `std::deque` of bytes — only a step above the previous one;

- [`boost::multiprecision`](https://www.boost.org/doc/libs/1_67_0/libs/multiprecision/doc/html/index.html)'s at the latest tag `v1.67.0` (their own provider, not GMP);


