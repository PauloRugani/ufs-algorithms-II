import base64
import zlib

with open("quicksort/cpp_code/quicksort.exe", "rb") as f:
    exe_bytes_list = list(f.read())

exe_bytes = bytes(exe_bytes_list)
compressed = zlib.compress(exe_bytes, level=9)
encoded = base64.b64encode(compressed).decode("utf-8")

with open('quicksort/compressed_bin.txt', 'w') as f:
    f.write(encoded)