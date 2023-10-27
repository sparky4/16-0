@echo ==== Making Sample RTC file ====
@ted2rtl scb sampcbat.rtc -r -c
@rem scb is extension name
@rem sampcbat.rtc is output name
@rem -r means for registered (-s would be shareware, but we suggest not making
@rem    shareware maps--you have to know what you can't use, like Dog Mode,
@rem    etc.)
@rem -c means Comm-bat levels (-l would be game levels) The only difference 
@rem    is, Comm-bat levels don't require an exit arch.  You can, of course, 
@rem    load regular game levels as Comm-bat levels.
