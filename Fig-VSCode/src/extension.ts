import * as path from 'path';
import { ExtensionContext, workspace } from 'vscode';
import {
    LanguageClient,
    LanguageClientOptions,
    ServerOptions
} from 'vscode-languageclient/node';

let client: LanguageClient;

export function activate(context: ExtensionContext) {
    const exeName = process.platform === 'win32' ? 'Fig-LSP.exe' : 'Fig-LSP';

    // 获取插件安装后的绝对沙箱路径
    const serverCommand = context.asAbsolutePath(path.join('bin', exeName));

    const serverOptions: ServerOptions = {
        run: { command: serverCommand, args: [] },
        debug: { command: serverCommand, args: [] }
    };

    const clientOptions: LanguageClientOptions = {
        documentSelector: [{ scheme: 'file', language: 'fig' }],
        synchronize: {
            fileEvents: workspace.createFileSystemWatcher('**/*.fig')
        }
    };

    client = new LanguageClient(
        'figLanguageServer',
        'Fig Language Server',
        serverOptions,
        clientOptions
    );

    client.start();
}

export function deactivate(): Thenable<void> | undefined {
    if (!client) {
        return undefined;
    }
    return client.stop();
}