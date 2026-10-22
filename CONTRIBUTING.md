
# Introduction
<!--冒頭の挨拶を記載します。[任意]-->
* First of all, thank you for contributing!   
* Please follow the guidelines below to contribute to the project.

## Disclaimer
<!--MELFA ROS2 Driverに関連するリポジトリを使用した際に発生する事象に三菱電機および著作者は一切責任を負わない旨を記載。[推奨]-->
1. lease note that the MELFA ROS 2 Driver has not received safety certification.
2. By using this driver, users acknowledge the potential for physical damage, personal injury, or destruction of hardware, and assume all responsibility for its use.
3. Users shall assume all risks and legal liabilities. Mitsubishi Electric Corporation and the developers shall not be held liable for any damage or injury whatsoever.
4. It is highly recommended to test in a simulation environment before executing any motions on the robot.

## Types of Contributions
<!--プロジェクトがどのような種類のコントリビューションを求めているのかを記載します。[推奨]-->
This project accepts the following categories of contributions.
1. **Bug Reports**:  If you find a bug, please create an Issue to report it. Provide as much detail as possible, including steps to reproduce the issue and relevant screenshots.
2. **Feature Requests**: If you have ideas for new features, please open an Issue for discussion. Please explain the details of the feature and its potential benefits/use cases.
3. **Code Contributions**:  If you would like to contribute code, please follow the workflow and steps described below.
4. **Documentation**:  Improving documentation is also a valuable contribution. This includes updating existing documents, adding new guides, or fixing typos.
5. **Testing**: You can contribute by testing the project and reporting any issues found. Writing unit tests or integration tests for the codebase is also highly appreciated.
6. **Security Vulnerabilities**: For security reasons, please do not report vulnerabilities via Issues or Pull Requests. Instead, please send them to our mailing list: melfa-ros2-contact@pc.MitsubishiElectric.co.jp

## Contribution Workflow
<!--コントリビューションの流れを記載します。[必須]-->
1. **Check/Create an Issue**: Search existing Issues to see if the same problem or proposal has already been reported. If not, create a new Issue and provide as much detail as possible.
2. **Fork the Repository**: Fork the repository to your GitHub account and clone it to your local machine.
    ```bash
    git clone https://github.com/Mitsubishi-Electric
    ```
3. **Create a branch**: Create a new branch for your work using a clear, descriptive name.
    ```bash
    git switch -c feature/feature-name
    ```
4. **Make changes**: Make your changes in the new branch. Please ensure your code follows the project's coding standards.
5. **Commit the changes**: Commit changes using a clear and descriptive commit message.
    ```bash
    git add <Staged changes>
    git commit -s -m "Add feature-name"
    ```
   Using the -s flag automatically appends the Signed-off-by: line required for the Developer Certificate of Origin (DCO) mentioned below.
6. **Push your changes**: Push the changes to your forked repository.
    ```bash
    git push origin feature/feature-name
    ```
7. **Create a Pull Request**: On GitHub, create a Pull Request against the original repository.Clearly describe your changes and link any related issues.

## Developer Certificate of Origin (DCO)
<!--DCOを利用する場合に記載します。[推奨]-->
This project adheres to the Developer Certificate of Origin (DCO).(https://developercertificate.org/)   
The DCO is a framework for contributors to verify that they have the right to submit their contributions and that they are doing so under the project's license terms.

[（Japanese Reference Translation）](https://developercertificate.opensource.jp/)

1 **Agreeing to the DCO**  
　By contributing to this project—whether through commits, Pull Requests,patches, or any other form—you agree to the terms of 　
the Developer Certificate of Origin (DCO).

2 **Appending the Signed-off-by traile**  
　All commits must contain a  `Signed-off-by:` line.
  ```bash
　Signed-off-by: Full Name <email address>
  ```
　Example: Signed-off-by: Taro Yamada <taro@example.com>

3 How to add
　Signed-off-by: The line is automatically appended when you use the following command.
  ```bash
　git commit -s
  ```
　or
  ```bash
　git commit --signoff
  ```
4 **Commits in a Pull Request**  
* A Signed-off-by: line is required for every commit included in a Pull Request.  
* When adding or amending commits, please double-check that each one still contains a valid sign-off.

5 **Important Notes**  
* When using code or text from third parties, please ensure you have the appropriate rights and permissions.  
* For joint works, please obtain the necessary consent from all co-authors.   
* If your contribution was created as part of your duties for a company or organization,  please verify that you are authorized to submit it.   
* Please note that the Signed-off-by: line does not constitute a transfer of copyright ownership.

## Issue and Pull Request Guidelines
<!--Issueとプルリクエストのガイドラインを記載します。[推奨]-->
- **Issue**: Please provide as much detail as possible for bug reports and feature suggestions. Including reproduction steps and screenshots is highly appreciated.
- **Pull Request**: Please link any related issue numbers and provide a concise description of your changes.
- GitHub provides additional documentation on [forking a repository](https://help.github.com/articles/fork-a-repo/) and
[creating a pull request](https://help.github.com/articles/creating-a-pull-request/).

## Communication
<!--コミュニケーションをとるための方法を記載します。[必須]-->
- Please leverage the Issues or Discussions tabs for any questions or project-related conversations.
- While we aim to respond as promptly as possible, please understand that it may take some time to provide a thorough reply. We appreciate your patience.

## Licensing
Any contribution that you make to this repository will
be under the Apache License, Version 2.0, as dictated by that
[license](http://www.apache.org/licenses/LICENSE-2.0).
