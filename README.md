# redbase
A mini relational database implementation from Stanford CS346 course project.

RedBase is a complete single-user relational database management system.
I focused to implement this project to run in Windows, contrary as proposed by CS346 rules (Must run in: Linux).

Pre-requisites:
Microsoft Visual Studio 2017
1. Windows SDK version: 10.0.16299.0
   - Select the project --> Right click --> Properties;
   - Guide General --> Windows SDK version --> Select the version.
2. Configuring the c++ libraries:
  - Select the project;
  - Right click --> Properties;
  - Click on Linker;
  - Additional Library Directories;
  - Click at the drop down --> Option Edit;
  - Click Add new line;
  - Add: C:\Program Files (x86)\Windows Kits\10\Include\10.0.16299.0\ucrt
3. Configure Visual Studio to use a custom build (win flex-bison):
   - Link to download: https://sourceforge.net/projects/winflexbison/
   - Extract the files and rename the folder (my case the folder was renamed to flex and I put it inside: C:\Program Files (x86)
   - Select the project;
   - Right click --> Build Dependencies --> Build Customizations...
   - Click on Find Existing --> Path to the extracted folder --> custom_build_rules --> select the win_flex_bison_custom_build.targets file --> Open;
   - If a window  pop up, please click in Yes;
   - Add the win_flex.exe directory to the windows path in System Variable.
   - Restart the applications which is to recognize the path.
     - Win flex-bison is a based project for Flex and Bison, with custom build rules provided:
       1. win_flex_bison_custom_build.props
       2. win_flex_bison_custom_build.targets
       3. win_flex_bison_custom_build.xml            

There are 3 data types available to the DBMS:
1. Integer --> Represented by the letter i;
2. String  --> Represented by the letter c followed by the size of the string (1 to 255);
3. Float   --> Represented by the letter f.

Each line must be ended by ";".
    
Some of basics syntaxes recognized by the code:

Create table:

    create table TableName (FieldName1 i, fieldName2 c30, fieldName3 f);


Query (Where what is inside [ is optional):

    Select * from TableName [where FieldNameN [[=], [!=], [>], [<], [>=], [<=]]]
    , or,
    Select [[FieldName1], [FieldName2], [FieldNameN]] from TableName [where FieldNameN [[=], [!=], [>], [<], [>=], [<=]]]
    ,or,
    print TableName;
    
Print a table structure:

    help TableName;
    
To list all tables in the database:

    help;
    
Insert:

    Insert into TableName values (FieldName1, "FieldName2", ...);

Delete:

    delete from TableName [where ];

Update:

    update TableName set FieldName = [];

Create Index:

    create index TableName(FieldName);
    
Drop Index:

    drop index TableName(FieldName);

Details about the project: https://web.stanford.edu/class/cs346/2015/redbase.html


TODO:
1. Improve the syntax error message, specifying the invalid character in a query;
2. Implement ORDER BY (asc: 1, none: 0, desc= -1);
3. Provide test project;

I hope that this code help who want to learn a bit more about the internals of database engines.
