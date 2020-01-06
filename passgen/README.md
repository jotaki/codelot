# A Password Generator

A password generator, in place of a password dictionary.

Use a seed password and specify length to create a custom password for your services.

Now with support for custom letters, useful for services that don't allow all
character sets.

## Help output
```bash
$ ./passgen -h
Valid options are:
-s <seed>    -- srand() seed value.
-m <modulus> -- modulus value.
-l <letters> -- letters.
-L           -- read letters from stdin.
-q           -- read letters using getpass().
             -- must be used with -L
-h           -- shows this help.
```
