import argparse
from random import SystemRandom

ESCAPE_CHARS = "\\\"?'"
CHARS_SPECIAL = "\\!@#$%^&*()_+{}:\"'<>?|[];,./`~"
CHARS_LOWER = "abcdefghijklmnopqrstuvwxyz"
CHARS_UPPER = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
CHARS_NUMBER = "0123456789"
CHARS = [
    CHARS_SPECIAL,
    CHARS_LOWER,
    CHARS_UPPER,
    CHARS_NUMBER
]


parser = argparse.ArgumentParser()
parser.add_argument('-l', '--length', type=int)
args = parser.parse_args()

length = args.length

pepper = ""
for _ in range(length):
    symbol_type = SystemRandom().randrange(len(CHARS))
    char_index= SystemRandom().randrange(len(CHARS[symbol_type]))
    char = CHARS[symbol_type][char_index]
    if char in ESCAPE_CHARS:
        pepper += "\\"
    
    pepper += char
    
print(pepper, end='')
