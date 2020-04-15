/*************************************************************************************

    Grid physics library, www.github.com/paboyle/Grid

    Source file: WilsonKernelsAsmBodyA64FX.h

    Copyright (C) 2020

Author: Nils Meyer <nils.meyer@ur.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    See the full license in the file "LICENSE" in the top level distribution directory
*************************************************************************************/
/*  END LEGAL */
#ifdef KERNEL_DAG
#define DIR0_PROJMEM(base) XP_PROJMEM(base);
#define DIR1_PROJMEM(base) YP_PROJMEM(base);
#define DIR2_PROJMEM(base) ZP_PROJMEM(base);
#define DIR3_PROJMEM(base) TP_PROJMEM(base);
#define DIR4_PROJMEM(base) XM_PROJMEM(base);
#define DIR5_PROJMEM(base) YM_PROJMEM(base);
#define DIR6_PROJMEM(base) ZM_PROJMEM(base);
#define DIR7_PROJMEM(base) TM_PROJMEM(base);
#define DIR0_RECON   XP_RECON
#define DIR1_RECON   YP_RECON_ACCUM
#define DIR2_RECON   ZP_RECON_ACCUM
#define DIR3_RECON   TP_RECON_ACCUM
#define DIR4_RECON   XM_RECON_ACCUM
#define DIR5_RECON   YM_RECON_ACCUM
#define DIR6_RECON   ZM_RECON_ACCUM
#define DIR7_RECON   TM_RECON_ACCUM
#else
#define DIR0_PROJMEM(base) XM_PROJMEM(base);
#define DIR1_PROJMEM(base) YM_PROJMEM(base);
#define DIR2_PROJMEM(base) ZM_PROJMEM(base);
#define DIR3_PROJMEM(base) TM_PROJMEM(base);
#define DIR4_PROJMEM(base) XP_PROJMEM(base);
#define DIR5_PROJMEM(base) YP_PROJMEM(base);
#define DIR6_PROJMEM(base) ZP_PROJMEM(base);
#define DIR7_PROJMEM(base) TP_PROJMEM(base);
#define DIR0_RECON   XM_RECON
#define DIR1_RECON   YM_RECON_ACCUM
#define DIR2_RECON   ZM_RECON_ACCUM
#define DIR3_RECON   TM_RECON_ACCUM
#define DIR4_RECON   XP_RECON_ACCUM
#define DIR5_RECON   YP_RECON_ACCUM
#define DIR6_RECON   ZP_RECON_ACCUM
#define DIR7_RECON   TP_RECON_ACCUM
#endif

//using namespace std;

#undef SHOW
//#define SHOW

#undef WHERE

#ifdef INTERIOR_AND_EXTERIOR
#define WHERE "INT_AND_EXT"
#endif

#ifdef INTERIOR
#define WHERE "INT"
#endif

#ifdef EXTERIOR
#define WHERE "EXT"
#endif

//#pragma message("here")



////////////////////////////////////////////////////////////////////////////////
// Comms then compute kernel
////////////////////////////////////////////////////////////////////////////////
#ifdef INTERIOR_AND_EXTERIOR

#define ASM_LEG(Dir,NxtDir,PERMUTE_DIR,PROJ,RECON)			\
      basep = st.GetPFInfo(nent,plocal); nent++;			\
      if ( local ) {							\
    /* PREFETCH_GAUGE_L1(Dir); slightly worse performance */ \
	PROJ(base);							\
    /* PREFETCH_GAUGE_L1(Dir); slightly worse performance */ \
	MAYBEPERM(PERMUTE_DIR,perm);					\
      } else {								\
	LOAD_CHI(base);							\
      }									\
      base = st.GetInfo(ptype,local,perm,NxtDir,ent,plocal); ent++;	\
      PREFETCH_CHIMU(base);						\
      MULT_2SPIN_DIR_PF(Dir,basep);					\
      PREFETCH_GAUGE_L1(NxtDir); \
      RECON;								\

#define ASM_LEG_XP(Dir,NxtDir,PERMUTE_DIR,PROJ,RECON)			\
  base = st.GetInfo(ptype,local,perm,Dir,ent,plocal); ent++;		\
  PF_GAUGE(Xp);								\
  PREFETCH1_CHIMU(base);						\
  ASM_LEG(Dir,NxtDir,PERMUTE_DIR,PROJ,RECON)

#define RESULT(base,basep) SAVE_RESULT(base,basep);

#endif

////////////////////////////////////////////////////////////////////////////////
// Pre comms kernel -- prefetch like normal because it is mostly right
////////////////////////////////////////////////////////////////////////////////
#ifdef INTERIOR

#define ASM_LEG(Dir,NxtDir,PERMUTE_DIR,PROJ,RECON)			\
      basep = st.GetPFInfo(nent,plocal); nent++;			\
      if ( local ) {							\
	PROJ(base);							\
	MAYBEPERM(PERMUTE_DIR,perm);					\
      }else if ( st.same_node[Dir] ) {LOAD_CHI(base);}			\
      if ( local || st.same_node[Dir] ) {				\
	MULT_2SPIN_DIR_PF(Dir,basep);					\
	RECON;								\
      }									\
      base = st.GetInfo(ptype,local,perm,NxtDir,ent,plocal); ent++;	\
      PREFETCH_CHIMU(base);						\

#define ASM_LEG_XP(Dir,NxtDir,PERMUTE_DIR,PROJ,RECON)			\
  base = st.GetInfo(ptype,local,perm,Dir,ent,plocal); ent++;		\
  PF_GAUGE(Xp);								\
  PREFETCH1_CHIMU(base);						\
  { ZERO_PSI; }								\
  ASM_LEG(Dir,NxtDir,PERMUTE_DIR,PROJ,RECON)

#define RESULT(base,basep) SAVE_RESULT(base,basep);

#endif
////////////////////////////////////////////////////////////////////////////////
// Post comms kernel
////////////////////////////////////////////////////////////////////////////////
#ifdef EXTERIOR


#define ASM_LEG(Dir,NxtDir,PERMUTE_DIR,PROJ,RECON)			\
  base = st.GetInfo(ptype,local,perm,Dir,ent,plocal); ent++;		\
  if((!local)&&(!st.same_node[Dir]) ) {					\
    LOAD_CHI(base);							\
    MULT_2SPIN_DIR_PF(Dir,base);					\
    RECON;								\
    nmu++;								\
  }

#define ASM_LEG_XP(Dir,NxtDir,PERMUTE_DIR,PROJ,RECON)			\
  nmu=0;								\
  { ZERO_PSI;}								\
  base = st.GetInfo(ptype,local,perm,Dir,ent,plocal); ent++;		\
  if((!local)&&(!st.same_node[Dir]) ) {					\
    LOAD_CHI(base);							\
    MULT_2SPIN_DIR_PF(Dir,base);					\
    RECON;								\
    nmu++;								\
  }

#define RESULT(base,basep) if (nmu){ ADD_RESULT(base,base);}

#endif
{
  int nmu;
  int local,perm, ptype;
  uint64_t base;
  uint64_t basep;
  const uint64_t plocal =(uint64_t) & in[0];

  MASK_REGS;
  int nmax=U.oSites();
  for(int site=0;site<Ns;site++) {
#ifndef EXTERIOR
    //    int sU =lo.Reorder(ssU);
    int sU =ssU;
    int ssn=ssU+1;     if(ssn>=nmax) ssn=0;
    //    int sUn=lo.Reorder(ssn);
    int sUn=ssn;
    LOCK_GAUGE(0);
#else
    int sU =ssU;
    int ssn=ssU+1;     if(ssn>=nmax) ssn=0;
    int sUn=ssn;
#endif
    for(int s=0;s<Ls;s++) {
      ss =sU*Ls+s;
      ssn=sUn*Ls+s;
      int  ent=ss*8;// 2*Ndim
      int nent=ssn*8;

      uint64_t delta_base, delta_base_p;

   ASM_LEG_XP(Xp,Yp,PERMUTE_DIR3,DIR0_PROJMEM,DIR0_RECON);

#ifdef SHOW
      float rescale = 64. * 12.;
      std::cout << "=================================================================" << std::endl;
      std::cout << "ss = " << ss << "   ssn = " << ssn << std::endl;
      std::cout << "sU = " << sU << "   ssU = " << ssU << std::endl;
      std::cout << " " << std::endl;


      std::cout << "Dir = " << Xp << "        "  << WHERE<< std::endl;

      std::cout << "ent  nent  local  perm       = " << ent << "  " << nent << "  " << local << "  "  << perm << std::endl;
      std::cout << "st.same_node[Dir] = " << st.same_node[Xp] << std::endl;
      std::cout << "base              = " << (base - plocal)/rescale << std::endl;
      std::cout << "Basep             = " << (basep - plocal)/rescale << std::endl;
      //printf("U                 = %llu\n", (uint64_t)&[sU](Dir));
      std::cout << "----------------------------------------------------" << std::endl;
#endif

      ASM_LEG(Yp,Zp,PERMUTE_DIR2,DIR1_PROJMEM,DIR1_RECON);

#ifdef SHOW
      std::cout << "Dir = " << Yp << "        "  << WHERE<< std::endl;

      std::cout << "ent  nent  local  perm       = " << ent << "  " << nent << "  " << local << "  "  << perm << std::endl;
      std::cout << "st.same_node[Dir] = " << st.same_node[Yp] << std::endl;
      std::cout << "base              = " << (base - plocal)/rescale << std::endl;
      std::cout << "Basep             = " << (basep - plocal)/rescale << std::endl;
      //printf("U                 = %llu\n", (uint64_t)&[sU](Dir));
      std::cout << "----------------------------------------------------" << std::endl;
#endif

      ASM_LEG(Zp,Tp,PERMUTE_DIR1,DIR2_PROJMEM,DIR2_RECON);

#ifdef SHOW
      std::cout << "Dir = " << Zp << "        "  << WHERE<< std::endl;

      std::cout << "ent  nent  local  perm       = " << ent << "  " << nent << "  " << local << "  "  << perm << std::endl;
      std::cout << "st.same_node[Dir] = " << st.same_node[Zp] << std::endl;
      std::cout << "base              = " << (base - plocal)/rescale << std::endl;
      std::cout << "Basep             = " << (basep - plocal)/rescale << std::endl;
      //printf("U                 = %llu\n", (uint64_t)&[sU](Dir));
      std::cout << "----------------------------------------------------" << std::endl;
#endif

      ASM_LEG(Tp,Xm,PERMUTE_DIR0,DIR3_PROJMEM,DIR3_RECON);

#ifdef SHOW
      std::cout << "Dir = " << Tp << "        "  << WHERE<< std::endl;

      std::cout << "ent  nent  local  perm       = " << ent << "  " << nent << "  " << local << "  "  << perm << std::endl;
      std::cout << "st.same_node[Dir] = " << st.same_node[Tp] << std::endl;
      std::cout << "base              = " << (base - plocal)/rescale << std::endl;
      std::cout << "Basep             = " << (basep - plocal)/rescale << std::endl;
      //printf("U                 = %llu\n", (uint64_t)&[sU](Dir));
      std::cout << "----------------------------------------------------" << std::endl;
#endif

      ASM_LEG(Xm,Ym,PERMUTE_DIR3,DIR4_PROJMEM,DIR4_RECON);

#ifdef SHOW
      std::cout << "Dir = " << Xm << "        "  << WHERE<< std::endl;

      std::cout << "ent  nent  local  perm       = " << ent << "  " << nent << "  " << local << "  "  << perm << std::endl;
      std::cout << "st.same_node[Dir] = " << st.same_node[Xm] << std::endl;
      std::cout << "base              = " << (base - plocal)/rescale << std::endl;
      std::cout << "Basep             = " << (basep - plocal)/rescale << std::endl;
      //printf("U                 = %llu\n", (uint64_t)&[sU](Dir));
      std::cout << "----------------------------------------------------" << std::endl;
#endif

      ASM_LEG(Ym,Zm,PERMUTE_DIR2,DIR5_PROJMEM,DIR5_RECON);

#ifdef SHOW
      std::cout << "Dir = " << Ym << "        "  << WHERE<< std::endl;

      std::cout << "ent  nent  local  perm       = " << ent << "  " << nent << "  " << local << "  "  << perm << std::endl;
      std::cout << "st.same_node[Dir] = " << st.same_node[Ym] << std::endl;
      std::cout << "base              = " << (base - plocal)/rescale << std::endl;
      std::cout << "Basep             = " << (basep - plocal)/rescale << std::endl;
      //printf("U                 = %llu\n", (uint64_t)&[sU](Dir));
      std::cout << "----------------------------------------------------" << std::endl;
#endif

      ASM_LEG(Zm,Tm,PERMUTE_DIR1,DIR6_PROJMEM,DIR6_RECON);

#ifdef SHOW
      std::cout << "Dir = " << Zm << "        "  << WHERE<< std::endl;

      std::cout << "ent  nent  local  perm       = " << ent << "  " << nent << "  " << local << "  "  << perm << std::endl;
      std::cout << "st.same_node[Dir] = " << st.same_node[Zm] << std::endl;
      std::cout << "base              = " << (base - plocal)/rescale << std::endl;
      std::cout << "Basep             = " << (basep - plocal)/rescale << std::endl;
      //printf("U                 = %llu\n", (uint64_t)&[sU](Dir));
      std::cout << "----------------------------------------------------" << std::endl;
#endif

      ASM_LEG(Tm,Xp,PERMUTE_DIR0,DIR7_PROJMEM,DIR7_RECON);

#ifdef SHOW
      std::cout << "Dir = " << Tm << "        "  << WHERE<< std::endl;

      std::cout << "ent  nent  local  perm       = " << ent << "  " << nent << "  " << local << "  "  << perm << std::endl;
      std::cout << "st.same_node[Dir] = " << st.same_node[Tm] << std::endl;
      std::cout << "base              = " << (base - plocal)/rescale << std::endl;
      std::cout << "Basep             = " << (basep - plocal)/rescale << std::endl;
      //printf("U                 = %llu\n", (uint64_t)&[sU](Dir));
      std::cout << "----------------------------------------------------" << std::endl;
#endif

#ifdef EXTERIOR
      if (nmu==0) break;
      //      if (nmu!=0) std::cout << "EXT "<<sU<<std::endl;
#endif
      base = (uint64_t) &out[ss];
      basep= st.GetPFInfo(nent,plocal); nent++;

#ifdef SHOW
      std::cout << "Dir = FINAL        " <<  WHERE<< std::endl;;

      base_ss = base;
      std::cout << "base              = " << (base - (uint64_t) &out[0])/rescale << std::endl;
      std::cout << "Basep             = " << (basep - plocal)/rescale << std::endl;
      //printf("U                 = %llu\n", (uint64_t)&[sU](Dir));
      std::cout << "----------------------------------------------------" << std::endl;
#endif

      //basep = (uint64_t) &out[ssn];
      RESULT(base,basep);
    }
    ssU++;
    UNLOCK_GAUGE(0);
  }
}

#undef DIR0_PROJMEM
#undef DIR1_PROJMEM
#undef DIR2_PROJMEM
#undef DIR3_PROJMEM
#undef DIR4_PROJMEM
#undef DIR5_PROJMEM
#undef DIR6_PROJMEM
#undef DIR7_PROJMEM
#undef DIR0_RECON
#undef DIR1_RECON
#undef DIR2_RECON
#undef DIR3_RECON
#undef DIR4_RECON
#undef DIR5_RECON
#undef DIR6_RECON
#undef DIR7_RECON
#undef ASM_LEG
#undef ASM_LEG_XP
#undef RESULT
