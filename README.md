# myOS
# Temp Memory Layout

<pre>
|           |
+-----------+
|           |   0x0010 0000
|  kernel   |
|   code    |     16 MiB
|           |
\           /
/           \
|           |   0x010f ffff
+-----------+
|           |   0x0110 0000
|    GDT    |
|   65536   |
|  entries  |     512 KiB (65536 x 8B)
|           |
\           /
/           \
|           |   0x0117 ffff
+-----------+
|           |   0x0118 0000
|    IDT    |
|    256    |     2 KiB (256 x 8B)
|  entries  |
|           |
\           /
/           \
|           |   0x0118 07ff
+-----------+
|           |   0x0118 0800
|           |
|  kernel   |
|  stack    |    1 MiB + 1B
\           /
/           \
|           |
|           |   0x0128 0800
+-----------+
|           |
</pre>
