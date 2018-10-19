Porting
-------
Thank you for finding interest in my software.
In order for the porting process to go smoothly, please read the following guidelines:
* The game is a work-in-progress and remains in active development. Please contact me before you make any changes - a port to your platform might be already in progress!
* If your platform's build system is GNU Make compatible, use the existing Makefile and add your platform to the PLATFORM variable.
* Set only the macros which are appropriate for your platform. For example, don't set "NO_FRAMELIMIT" if your platform has no built-in frame limiter.
* The best way to integrate your changes into the upstream repository is to send me a merge request via GitHub.
* Keep all commit messages informative and uniform.
* All code contributions must compile without warnings with DEBUG=1 flag.
* Please mind the license of the source code and game assets when you prepare a public release.

Contact
-------
You can reach me at the following e-mail address:
* contact at artur-rojek.eu
