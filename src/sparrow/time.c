#include <u.h>
#include <avian.h>
#include <errno.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <time.h>

void
usage(void) {
  fprint(stderr, "usage: time [-p] utility [argument...]\n");
  exit(1);
}

int
main(int argc, char *argv[]) {
  double clk_tck;
  int pflag;
  ulong st_time, en_time;
  struct tms st_cpu, en_cpu;

  pflag = 0;
  ARGBEGIN("p"){
  case 'p':
    pflag = 1;
    break;
  default:
    usage();
  }ARGEND 
  
  if(argc < 1)
    usage();
  st_time = times(&st_cpu);
  if(fork() == 0) {
    execv(*argv, argv);
    alert("time: can't exec %s: %m", *argv);
    _exit(errno == ENOENT ? 127 : 126);
  }
  wait(&rval);
  en_time = times(&en_cpu);
  clk_tck = sysconf(_SC_CLK_TCK);
  fprintf(stderr, pflag ? "real %9.2f\nuser %9.2f\nsys  %9.2f\n" :
    "%9.2f real %9.2f user %9.2f sys\n",
    (double)(en_time-st_time)/clk_tck,
    (double)en_cpu.tms_cutime/clk_tck,
    (double)en_cpu.tms_cstime/clk_tck);
  exit(WEXITSTATUS(rval));
}
