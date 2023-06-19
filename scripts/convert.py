#!/usr/bin/env python3

import itertools
import argparse
from typing import Iterator

header_file_template = \
"""#pragma once

const char {}[] = {{
{}
}};"""

def batched(iterable: bytes, n: int) -> Iterator[bytes]:
    """
    Batch data into tuples of length n. The last batch may be shorter.
    
    :param iterable: The iterable providing the elements that will be batched
    :param n: The size of the batch

    :raises ValueError: If n is less that 1

    :return: An iterator producing batches from input iterable.
    """
    if n < 1:
        raise ValueError('n must be at least one')
    it = iter(iterable)
    yield bytes([b for b in itertools.islice(it, n)])


def to_char_array(bytes: bytes) -> str:
    """
    Returns a string containing each byte as an escaped C character in quotes and separated with commas.
    Ex: b"012" -> "'\x30', '\x31', '\x32'"
    
    :param bytes: The input bytes to convert

    :return: The string corresponding to the input bytes
    """
    return ", ".join([f"'\\x{b:02x}'" for b in bytes])


def to_batched_char_array(bytes: bytes, column_count: int) -> str:
    """
    Returns a string containing multiple lines. Each of these lines contains at most n bytes as escaped
    C characters in quotes and separated with commas.
    Ex: b"012", n = 2 -> "'\x30', '\x31'\n    '\x32'"
    
    :param bytes: The input bytes to convert

    :return: The string corresponding to the input bytes
    """
    return ",\n".join(["    " + to_char_array(batch) for batch in batched(bytes, column_count)])


def main():
    parser = argparse.ArgumentParser(description="Generate a C++ header file containing the binary representation of the input file")

    parser.add_argument("--output-header", required=True, type=str, help="The output file name")
    parser.add_argument("--input-file", required=True, type=str, help="The input file to embed in the header file")
    parser.add_argument("--array-name", required=True, type=str, help="The name of the array to use in the header file")
    parser.add_argument("--columns", required=False, type=int, help="The number of column to use for the array")

    args = parser.parse_args()

    with open(args.output_header, "w") as output_file, open(args.input_file, "rb") as input_file:
        data = input_file.read()
        output_file.write(header_file_template.format(args.array_name, to_batched_char_array(data, args.columns)))

if __name__ == "__main__":
    main()
