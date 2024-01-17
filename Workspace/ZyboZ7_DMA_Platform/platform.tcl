# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct /home/ozorakobo/Zybo-Z7/Zybo-Z7_Project/Workspace/ZyboZ7_DMA_Platform/platform.tcl
# 
# OR launch xsct and run below command.
# source /home/ozorakobo/Zybo-Z7/Zybo-Z7_Project/Workspace/ZyboZ7_DMA_Platform/platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {ZyboZ7_DMA_Platform}\
-hw {/home/ozorakobo/Zybo-Z7/Zybo-Z7_Project/ZyboZ7_design_wrapper.xsa}\
-proc {ps7_cortexa9_0} -os {standalone} -out {/home/ozorakobo/Zybo-Z7/Zybo-Z7_Project/Workspace}

platform write
platform generate -domains 
platform generate
domain create -name {Linux_Domain} -os {linux} -proc {ps7_cortexa9} -arch {32-bit} -display-name {Linux_Domain} -desc {} -runtime {cpp}
platform write
domain config -image {}
domain -report -json
platform write
domain config -runtime {cpp}
platform write
domain config -bootmode {sd}
platform write
platform generate -domains 
platform generate
