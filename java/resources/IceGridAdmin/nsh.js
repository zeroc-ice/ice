   function doResize() { 
     var clheight, headheight;
     if (self.innerHeight) // all except Explorer 
     { clheight = self.innerHeight; } 
     else if (document.documentElement && document.documentElement.clientHeight) // Explorer 6 Strict Mode 
     { clheight = document.documentElement.clientHeight; } 
     else if (document.body) // other Explorers 
     { clheight = document.body.clientHeight; } 
     headheight = document.getElementById('idheader').clientHeight;
     if (clheight < headheight ) {clheight = headheight + 1;}
     document.getElementById('idcontent').style.height = clheight - document.getElementById('idheader').clientHeight +'px'; 
   } 

   function nsrInit() { 
     contentbody = document.getElementById('idcontent'); 
     if (contentbody) { 
       contentbody.className = 'nonscroll'; 
       document.getElementsByTagName('body')[0].className = 'nonscroll'; 
       document.getElementsByTagName('html')[0].className = 'nonscroll'; 
       window.onresize = doResize; 
       doResize(); 
      } 
   } 
 