pipeline {
  agent any
  stages {
    stage('Init') {
      steps {
        echo "Testing..."
      }
    }
    stage('Build') {
      steps {
        echo "Building..."
        hostname
        git --version
        gcc --version
        g++ --version
      }
    }
    stage('Deploy') {
      steps {
        echo "Code deployed."
      }
    }
  }
}
