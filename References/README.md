# References

This section contains information about sources that were used in this project that came from external sources.  
The two C files you see listed below were the two candidates for inclusion in this project. The one chosen for
the interface to the hardware for video streaming was *vl2_capture.c*.  I'm keeping the other one around if any 
issues come up with capture.c (none have thus far).   

**v4l2_capture.c**

This main program came from the URL's below, and made into a C function which is integrated into the C++ code base. 
The file here is in its raw form (unchanged) from:     
     
*https://www.kernel.org/doc/html/v4.9/media/uapi/v4l/capture.c.html*    
    See also: 
*https://www.kernel.org/doc/html/v4.9/media/uapi/v4l/capture-example.html*     
     
Please see the source file for the appropriate copyright information.     
     
     
**v4l2_grab.c**

This main program came from the URL's below. It has not been used in this project (yet). 
The file here is in its raw form (unchanged) from:     
     
*https://www.kernel.org/doc/html/v4.9/media/uapi/v4l/v4l2grab.c.html*     
    See also:     
*https://www.kernel.org/doc/html/v4.9/media/uapi/v4l/v4l2grab-example.html*   
     
Please see the source file for the appropriate copyright information.     
     
     
**LoggerCpp**     
     
The logger used in this project comes from Github, courtesy of Sébastien Rombauts, who seems to write C++ code that
I can follow very easily (thank you).    
     
See the README under *...Samples/source/3rdparty/LoggerCpp/* for a link to the Github repository for this project.     
     
The script to build the library and install header files in the *Samples* project (this one) is in:     
*...Samples/source/3rdparty/LoggerCpp/localbuild.sh*.   
    
The cloned LoggerCpp project from Github, as used is this project is here (unchanged):     
*...Samples/source/3rdparty/LoggerCpp/SRombauts-LoggerCpp-a0868a8/*   
    
Please see the source file 
*...Samples/source/3rdparty/LoggerCpp/SRombauts-LoggerCpp-a0868a8/LICENSE.txt* 
for the appropriate copyright information for LoggerCpp.     

