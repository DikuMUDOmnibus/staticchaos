CC      = gcc
PROF    = -g
NOCRYPT =
C_FLAGS = -O -Wall $(PROF) $(NOCRYPT)
L_FLAGS = -O $(PROF)

O_FILES = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o bit.o board.o clan.o  \
          comm.o const.o db.o fight.o fist.o handler.o interp.o mazoku.o magic.o     \
	  mem.o mob_commands.o mob_prog.o olc.o olc_act.o olc_save.o patryn.o        \
	  quest.o saiyan.o save.o special.o soldier.o sorcerer.o string.o suit.o     \
	  update.o 

chaosium: $(O_FILES)
	rm -f chaosium 
	$(CC) $(L_FLAGS) -o chaosium $(O_FILES)

.c.o: merc.h
	$(CC) -c $(C_FLAGS) $<

clean:
	rm -f *.o chaosium
