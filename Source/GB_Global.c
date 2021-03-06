//------------------------------------------------------------------------------
// GB_Global: global values in GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// All Global storage is declared, initialized, and accessed here.  The
// contents of the GB_Global struct are only accessible to functions in this
// file.  Global storage is used to record a list of matrices with pending
// operations (for GrB_wait), to keep track of the GraphBLAS mode (blocking or
// non-blocking), for pointers to malloc/calloc/realloc/free functions, global
// matrix options, and other settings.

#include "GB.h"

//------------------------------------------------------------------------------
// Global storage: for all threads in a user application that uses GraphBLAS
//------------------------------------------------------------------------------

typedef struct
{

    //--------------------------------------------------------------------------
    // queue of matrices with work to do
    //--------------------------------------------------------------------------

    // In non-blocking mode, GraphBLAS needs to keep track of all matrices that
    // have pending operations that have not yet been finished.  In the current
    // implementation, these are matrices with pending tuples from
    // GrB_setElement, GxB_subassign, and GrB_assign that haven't been added to
    // the matrix yet.

    // A matrix with no pending tuples is not in the list.  When a matrix gets
    // its first pending tuple, it is added to the list.  A matrix is removed
    // from the list when another operation needs to use the matrix; in that
    // case the pending tuples are assembled for just that one matrix.  The
    // GrB_wait operation iterates through the entire list and assembles all
    // the pending tuples for all the matrices in the list, leaving the list
    // empty.  A simple link list suffices for the list.  The links are in the
    // matrices themselves so no additional memory needs to be allocated.  The
    // list never needs to be searched; if a particular matrix is to be removed
    // from the list, the GraphBLAS operation already been given the matrix
    // handle, and the prev & next pointers it contains.  All of these
    // operations can thus be done in O(1) time, except for GrB_wait which
    // needs to traverse the whole list once and then the list is empty
    // afterwards.

    // The access of these variables must be protected in a critical section.

    void *queue_head ;          // head pointer to matrix queue

    GrB_Mode mode ;             // GrB_NONBLOCKING or GrB_BLOCKING

    bool GrB_init_called ;      // true if GrB_init already called

    int nthreads_max ;          // max number of threads to use
    double chunk ;              // chunk size for determining # threads to use

    //--------------------------------------------------------------------------
    // hypersparsity and CSR/CSC format control
    //--------------------------------------------------------------------------

    double hyper_ratio ;        // default hyper_ratio for new matrices
    bool is_csc ;               // default CSR/CSC format for new matrices

    //--------------------------------------------------------------------------
    // abort function: only used for debugging
    //--------------------------------------------------------------------------

    void (* abort_function ) (void) ;

    //--------------------------------------------------------------------------
    // malloc/calloc/realloc/free: memory management functions
    //--------------------------------------------------------------------------

    // All threads must use the same malloc/calloc/realloc/free functions.
    // They default to the ANSI C11 functions, but can be defined by GxB_init.

    void * (* malloc_function  ) (size_t)         ;
    void * (* calloc_function  ) (size_t, size_t) ;
    void * (* realloc_function ) (void *, size_t) ;
    void   (* free_function    ) (void *)         ;
    bool malloc_is_thread_safe ;   // default is true

    //--------------------------------------------------------------------------
    // memory usage tracking: for testing and debugging only
    //--------------------------------------------------------------------------

    // malloc_tracking:  default is false.  There is no user-accessible API for
    // setting this to true.  If true, the following statistics are computed.
    // If false, all of the following are unused.

    // nmalloc:  To aid in searching for memory leaks, GraphBLAS keeps track of
    // the number of blocks of allocated that have not yet been freed.  The
    // count starts at zero.  GB_malloc_memory and GB_calloc_memory increment
    // this count, and free (of a non-NULL pointer) decrements it.  realloc
    // increments the count it if is allocating a new block, but it does this
    // by calling GB_malloc_memory.

    // inuse: the # of bytes currently in use by all threads

    // maxused: the max value of inuse since the call to GrB_init

    // malloc_debug: this is used for testing only (GraphBLAS/Tcov).  If true,
    // then use malloc_debug_count for testing memory allocation and
    // out-of-memory conditions.  If malloc_debug_count > 0, the value is
    // decremented after each allocation of memory.  If malloc_debug_count <=
    // 0, the GB_*_memory routines pretend to fail; returning NULL and not
    // allocating anything.

    bool malloc_tracking ;          // true if allocations are being tracked
    int64_t nmalloc ;               // number of blocks allocated but not freed
    bool malloc_debug ;             // if true, test memory handling
    int64_t malloc_debug_count ;    // for testing memory handling
    int64_t inuse ;                 // memory space current in use
    int64_t maxused ;               // high water memory usage

    //--------------------------------------------------------------------------
    // for testing and development
    //--------------------------------------------------------------------------

    int64_t hack ;                  // ad hoc setting (for draft versions only)
    bool burble ;                   // controls GBBURBLE output

    //--------------------------------------------------------------------------
    // for MATLAB interface only
    //--------------------------------------------------------------------------

    bool print_one_based ;          // if true, print 1-based indices
    int print_format ;              // for printing values

}
GB_Global_struct ;

GB_PUBLIC GB_Global_struct GB_Global ;

GB_Global_struct GB_Global =
{

    // queued matrices with work to do
    .queue_head = NULL,         // pointer to first queued matrix

    // GraphBLAS mode
    .mode = GrB_NONBLOCKING,    // default is nonblocking

    // initialization flag
    .GrB_init_called = false,   // GrB_init has not yet been called

    // max number of threads and chunk size
    .nthreads_max = 1,
    .chunk = GB_CHUNK_DEFAULT,

    // default format
    .hyper_ratio = GB_HYPER_DEFAULT,
    .is_csc = (GB_FORMAT_DEFAULT != GxB_BY_ROW),    // default is GxB_BY_ROW

    // abort function for debugging only
    .abort_function   = abort,

    // malloc/calloc/realloc/free functions: default to ANSI C11 functions
    .malloc_function  = malloc,
    .calloc_function  = calloc,
    .realloc_function = realloc,
    .free_function    = free,
    .malloc_is_thread_safe = true,

    // malloc tracking, for testing, statistics, and debugging only
    .malloc_tracking = false,
    .nmalloc = 0,                // memory block counter
    .malloc_debug = false,       // do not test memory handling
    .malloc_debug_count = 0,     // counter for testing memory handling
    .inuse = 0,                  // memory space current in use
    .maxused = 0,                // high water memory usage

    // for testing and development
    .hack = 0,
    .burble = false,

    // for MATLAB interface only
    .print_one_based = false,       // if true, print 1-based indices
    .print_format = 0               // for printing values
} ;

//==============================================================================
// GB_Global access functions
//==============================================================================

//------------------------------------------------------------------------------
// queue_head
//------------------------------------------------------------------------------

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
void GB_Global_queue_head_set (void *p)
{ 
    GB_Global.queue_head = p ;
}

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
void *GB_Global_queue_head_get (void)
{ 
    return (GB_Global.queue_head) ;
}

//------------------------------------------------------------------------------
// mode
//------------------------------------------------------------------------------

void GB_Global_mode_set (GrB_Mode mode)
{ 
    GB_Global.mode = mode ;
}

GrB_Mode GB_Global_mode_get (void)
{ 
    return (GB_Global.mode) ;
}

//------------------------------------------------------------------------------
// GrB_init_called
//------------------------------------------------------------------------------

GB_PUBLIC   // accessed by the MATLAB interface only
void GB_Global_GrB_init_called_set (bool GrB_init_called)
{ 
    GB_Global.GrB_init_called = GrB_init_called ;
}

GB_PUBLIC   // accessed by the MATLAB interface only
bool GB_Global_GrB_init_called_get (void)
{ 
    return (GB_Global.GrB_init_called) ;
}

//------------------------------------------------------------------------------
// nthreads_max
//------------------------------------------------------------------------------

GB_PUBLIC   // accessed by the MATLAB interface only
void GB_Global_nthreads_max_set (int nthreads_max)
{ 
    GB_Global.nthreads_max = GB_IMAX (nthreads_max, 1) ;
}

GB_PUBLIC   // accessed by the MATLAB interface only
int GB_Global_nthreads_max_get (void)
{ 
    return (GB_Global.nthreads_max) ;
}

//------------------------------------------------------------------------------
// OpenMP max_threads
//------------------------------------------------------------------------------

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
int GB_Global_omp_get_max_threads (void)
{ 
    return (GB_OPENMP_MAX_THREADS) ;
}

//------------------------------------------------------------------------------
// chunk
//------------------------------------------------------------------------------

GB_PUBLIC   // accessed by the MATLAB interface only
void GB_Global_chunk_set (double chunk)
{ 
    if (chunk <= GxB_DEFAULT) chunk = GB_CHUNK_DEFAULT ;
    GB_Global.chunk = fmax (chunk, 1) ;
}

GB_PUBLIC   // accessed by the MATLAB interface only
double GB_Global_chunk_get (void)
{ 
    return (GB_Global.chunk) ;
}

//------------------------------------------------------------------------------
// hyper_ratio
//------------------------------------------------------------------------------

void GB_Global_hyper_ratio_set (double hyper_ratio)
{ 
    GB_Global.hyper_ratio = hyper_ratio ;
}

double GB_Global_hyper_ratio_get (void)
{ 
    return (GB_Global.hyper_ratio) ;
}

//------------------------------------------------------------------------------
// is_csc
//------------------------------------------------------------------------------

void GB_Global_is_csc_set (bool is_csc)
{ 
    // FILE *f = NULL ;
    // GB_Context Context = NULL ;
    GB_Global.is_csc = is_csc ;
    // GBPR ("set GB_Global.is_csc to %d\n", (int) GB_Global.is_csc) ;
}

bool GB_Global_is_csc_get (void)
{ 
    // FILE *f = NULL ;
    // GB_Context Context = NULL ;
    // GBPR ("get GB_Global.is_csc = %d\n", (int) GB_Global.is_csc) ;
    return (GB_Global.is_csc) ;
}

//------------------------------------------------------------------------------
// abort_function
//------------------------------------------------------------------------------

GB_PUBLIC   // accessed by the MATLAB interface only
void GB_Global_abort_function_set (void (* abort_function) (void))
{ 
    GB_Global.abort_function = abort_function ;
}

GB_PUBLIC   // accessed by the MATLAB interface only
void GB_Global_abort_function (void)
{
    GB_Global.abort_function ( ) ;
}

//------------------------------------------------------------------------------
// malloc_function
//------------------------------------------------------------------------------

void GB_Global_malloc_function_set (void * (* malloc_function) (size_t))
{ 
    GB_Global.malloc_function = malloc_function ;
}

void * GB_Global_malloc_function (size_t size)
{ 
    bool ok = true ;
    void *p = NULL ;
    if (GB_Global.malloc_is_thread_safe)
    {
        p = GB_Global.malloc_function (size) ;
    }
    else
    {
        #define GB_CRITICAL_SECTION                             \
        {                                                       \
            p = GB_Global.malloc_function (size) ;              \
        }
        #include "GB_critical_section.c"
    }
    return (ok ? p : NULL) ;
}

//------------------------------------------------------------------------------
// calloc_function
//------------------------------------------------------------------------------

void GB_Global_calloc_function_set (void * (* calloc_function) (size_t, size_t))
{ 
    GB_Global.calloc_function = calloc_function ;
}

void * GB_Global_calloc_function (size_t count, size_t size)
{ 
    bool ok = true ;
    void *p = NULL ;
    if (GB_Global.malloc_is_thread_safe)
    {
        p = GB_Global.calloc_function (count, size) ;
    }
    else
    {
        #undef  GB_CRITICAL_SECTION
        #define GB_CRITICAL_SECTION                             \
        {                                                       \
            p = GB_Global.calloc_function (count, size) ;       \
        }
        #include "GB_critical_section.c"
    }
    return (ok ? p : NULL) ;
}

//------------------------------------------------------------------------------
// realloc_function
//------------------------------------------------------------------------------

void GB_Global_realloc_function_set
(
    void * (* realloc_function) (void *, size_t)
)
{ 
    GB_Global.realloc_function = realloc_function ;
}

void * GB_Global_realloc_function (void *p, size_t size)
{ 
    bool ok = true ;
    void *pnew = NULL ;
    if (GB_Global.malloc_is_thread_safe)
    {
        pnew = GB_Global.realloc_function (p, size) ;
    }
    else
    {
        #undef  GB_CRITICAL_SECTION
        #define GB_CRITICAL_SECTION                             \
        {                                                       \
            pnew = GB_Global.realloc_function (p, size) ;       \
        }
        #include "GB_critical_section.c"
    }
    return (ok ? pnew : NULL) ;
}

//------------------------------------------------------------------------------
// free_function
//------------------------------------------------------------------------------

void GB_Global_free_function_set (void (* free_function) (void *))
{ 
    GB_Global.free_function = free_function ;
}

void GB_Global_free_function (void *p)
{ 
    #if defined (USER_POSIX_THREADS) || defined (USER_ANSI_THREADS)
    bool ok = true ;
    #endif
    if (GB_Global.malloc_is_thread_safe)
    {
        GB_Global.free_function (p) ;
    }
    else
    {
        #undef  GB_CRITICAL_SECTION
        #define GB_CRITICAL_SECTION                             \
        {                                                       \
            GB_Global.free_function (p) ;                       \
        }
        #include "GB_critical_section.c"
    }
}

//------------------------------------------------------------------------------
// malloc_is_thread_safe
//------------------------------------------------------------------------------

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
void GB_Global_malloc_is_thread_safe_set (bool malloc_is_thread_safe)
{ 
    GB_Global.malloc_is_thread_safe = malloc_is_thread_safe ;
}

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
bool GB_Global_malloc_is_thread_safe_get (void)
{ 
    return (GB_Global.malloc_is_thread_safe) ;
}

//------------------------------------------------------------------------------
// malloc_tracking
//------------------------------------------------------------------------------

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
void GB_Global_malloc_tracking_set (bool malloc_tracking)
{ 
    GB_Global.malloc_tracking = malloc_tracking ;
}

bool GB_Global_malloc_tracking_get (void)
{ 
    return (GB_Global.malloc_tracking) ;
}

//------------------------------------------------------------------------------
// nmalloc
//------------------------------------------------------------------------------

void GB_Global_nmalloc_clear (void)
{ 
    GB_Global.nmalloc = 0 ;
}

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
int64_t GB_Global_nmalloc_get (void)
{ 
    return (GB_Global.nmalloc) ;
}

int64_t GB_Global_nmalloc_increment (void)
{ 
    return (++(GB_Global.nmalloc)) ;
}

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
int64_t GB_Global_nmalloc_decrement (void)
{ 
    return (--(GB_Global.nmalloc)) ;
}

//------------------------------------------------------------------------------
// malloc_debug
//------------------------------------------------------------------------------

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
void GB_Global_malloc_debug_set (bool malloc_debug)
{ 
    GB_Global.malloc_debug = malloc_debug ;
}

bool GB_Global_malloc_debug_get (void)
{ 
    return (GB_Global.malloc_debug) ;
}

//------------------------------------------------------------------------------
// malloc_debug_count
//------------------------------------------------------------------------------

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
void GB_Global_malloc_debug_count_set (int64_t malloc_debug_count)
{ 
    GB_Global.malloc_debug_count = malloc_debug_count ;
}

bool GB_Global_malloc_debug_count_decrement (void)
{ 
    return (GB_Global.malloc_debug_count-- <= 0) ;
}

//------------------------------------------------------------------------------
// inuse and maxused
//------------------------------------------------------------------------------

void GB_Global_inuse_clear (void)
{ 
    GB_Global.inuse = 0 ;
    GB_Global.maxused = 0 ;
}

void GB_Global_inuse_increment (int64_t s)
{ 
    GB_Global.inuse += s ;
    GB_Global.maxused = GB_IMAX (GB_Global.maxused, GB_Global.inuse) ;
}

void GB_Global_inuse_decrement (int64_t s)
{ 
    GB_Global.inuse -= s ;
}

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
int64_t GB_Global_inuse_get (void)
{ 
    return (GB_Global.inuse) ;
}

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
int64_t GB_Global_maxused_get (void)
{ 
    return (GB_Global.maxused) ;
}

//------------------------------------------------------------------------------
// hack: for setting an internal value for development only
//------------------------------------------------------------------------------

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
void GB_Global_hack_set (int64_t hack)
{ 
    GB_Global.hack = hack ;
}

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
int64_t GB_Global_hack_get (void)
{ 
    return (GB_Global.hack) ;
}

//------------------------------------------------------------------------------
// burble: for controlling the burble output
//------------------------------------------------------------------------------

void GB_Global_burble_set (bool burble)
{ 
    GB_Global.burble = burble ;
}

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
bool GB_Global_burble_get (void)
{ 
    return (GB_Global.burble) ;
}

//------------------------------------------------------------------------------
// for MATLAB interface only
//------------------------------------------------------------------------------

GB_PUBLIC   // accessed by the MATLAB interface only
void GB_Global_print_one_based_set (bool onebased)
{ 
    GB_Global.print_one_based = onebased ;
}

GB_PUBLIC   // accessed by the MATLAB interface only
bool GB_Global_print_one_based_get (void)
{ 
    return (GB_Global.print_one_based) ;
}

GB_PUBLIC   // accessed by the MATLAB interface only
void GB_Global_print_format_set (int f)
{ 
    GB_Global.print_format = f ;
}

GB_PUBLIC   // accessed by the MATLAB interface only
int GB_Global_print_format_get (void)
{ 
    return (GB_Global.print_format) ;
}

