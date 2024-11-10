import deflate
import sys

def main():
    s = bytes(sys.argv[1], "ascii")
    print("original data:", ", ".join([hex(b) for b in s]))
    print("compressed:   ", ", ".join([hex(b) for b in deflate.deflate_compress(s, 12)]))


if __name__ == "__main__":
    main()
