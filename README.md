# bsreader
betstick's reader if you're feeling polite!

# About
Intended to be a safer, faster, and more capable utility compared to the standard fread() set of functions. Higher speed is achieved using a memory buffer that is read to in chunks specified by the application. This buffer can be of any size as it sits in heap memory and is fully managed by the BSReader class. Reads that fall outside of the buffer are handled and the buffer is filled with the relevant data needed for reading to continue.

# Features
A memory buffer of arbitrary size. This should speed up reads on slow storage devices at the cost of some memory.


"Step in" and "Step out" functionality. Allows you to "follow" addresses without losing your original place. Calling stepIn($offset) will send you to the specified location and save your current header location to an internal stack. Calling stepOut() will return you to the most recent header location you stepped in from and remove it from the stack. Like how C# and the 010 editor work.


It is pretty simple and I think only relies on standard libraries? At least on Linux.

# Warnings
Calling stepOut() without having called stepIn() prior is undefined. Not sure what'll happen. Don't do it.


This tool is unaware of file modification time. If the data or filesize change without you recreating the BSReader object, expect crashes and/or corrupt reads. Might add a reinit later to "patch over" this. For now, destroy and recreate your reader object if the file has been modified in any way to ensure stable behavior. Or just don't modify it in the first place.