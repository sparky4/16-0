@echo ==== Making Sample RTL file ====
@ted2rtl sam sample.rtl -r -l
@rem sam is extension name
@rem sample.rtl is output name
@rem -r means for registered (-s would be shareware, but we suggest not making
@rem    shareware maps--you have to know what you can't use, like Dog Mode,
@rem    etc.)
@rem -l means game levels (-c would be Comm-bat levels) The only difference 
@rem    is, Comm-bat levels don't require an exit arch.  You can, of course, 
@rem    load regular game levels as Comm-bat levels.
