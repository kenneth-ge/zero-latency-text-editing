## low latency text editor...

So, Google Docs sucks. At least, when it comes to keyboard latency. In fact, on my slow potato of a laptop, Google Docs is so slow that it struggles to keep up with my typing, and I can see text appearing on my screen noticeably after typing it on my keyboard. 

But, while this keyboard lag is better in other word processors, like Microsoft Word or even your classic text editor, no matter where you go, it's still kind of mid. In fact, because of all the extra features and complexity we're adding to our software, [keyboard latency is actually worse now than it was for the Apple IIe](https://danluu.com/keyboard-latency/). Yikes. 

And sometimes, this performance tax we pay doesn't even give us useful features. Aside from the fact that most users rarely touch the advanced options in their favorite word processor, fundamentally, a lot of the extra bloat we're seeing is actually the result of developers adding in tools and code that make their jobs easier, but that ultimately result in end users paying the price. In codespeak, an example is frameworks like Node.js and Express -- their easy syntax and extra features allow us to develop cool websites and REST APIs faster than we ever could before -- but at the same time, they're slower, and it takes more computing power to serve the same number of users. 

Can we do better?

![image](https://github.com/kenneth-ge/zero-latency-text-editing/assets/57784063/3edac1c0-64fa-47a3-90b8-9462ceceffae)

With this project, I attempted to go pretty low-level (as far as I could reasonably go before hitting kernel space!), to see if I could hit the theoretical limit for how fast typing can get. I know, my wrists are destroyed too. But it was worth it!

## How does this work:
* Single buffering -- to delete, we scissor a small region. To add, we simply stamp our character
* Font glyph caching -- generating a ton of font glyphs all at once, cache them for later instead of rerendering like many other projects seem to do
* O(1) data structures to store and handle text
* Single-threaded -- asynchronous events introduce latency. The caret doesn't blink for this reason (and I wonder if the Sublime caret doesn't blink for this reason as well)
* Low-level languages and libraries (FreeType, OpenGL, C++)

Note that the retro font is not at all related to performance -- just thought it would be cool :P

## Features that need development:
* Word wrapping
* Open/save file dialog
* Support for certain Intel/Nvidia graphics cards (currently only known to work on AMD)
