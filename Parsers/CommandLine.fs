namespace Parsers
open System
open System.Windows.Forms

type CommandLine() =
    static member val useCompanionWindow = true with get,set
    static member val dxDebug = false with get,set

    static member Parse(arguments:String seq) =
        let arguments = List.ofSeq arguments
        let rec Parse(arguments) =
            match arguments with
            | "--help"::_ | "/?"::_ ->
                let message =
                    [   "-noCompanionWindow"
                        "    Do not display a companion window on the desktop while running"
                        ""
                        "-dxdebug"
                        "    Use DirectX debug mode"
                    ] |> String.concat "\n"
                MessageBox.Show (message,"Usage:") |> ignore
                Environment.Exit 0
            | "-noCompanionWindow"::tail ->
                CommandLine.useCompanionWindow <- false
                Parse tail
             | "-dxdebug"::tail ->
                CommandLine.dxDebug <- false
                Parse tail
            | unrecognizedArgument::_ ->
                failwithf "Unrecognized arguement: %s" unrecognizedArgument
            | _ -> ()
        Parse arguments
