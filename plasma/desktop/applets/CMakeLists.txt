add_subdirectory(kickoff)
add_subdirectory(trash)

if(KActivities_FOUND)
   add_subdirectory(showActivityManager)
endif(KActivities_FOUND)

if(NOT WIN32)
    #task and windowlist depend on libs/taskmanager
   add_subdirectory(tasks)
   add_subdirectory(windowlist)

   add_subdirectory(pager)
endif(NOT WIN32)
